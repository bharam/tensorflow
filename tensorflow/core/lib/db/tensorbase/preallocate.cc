/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <clocale>
#include <cstdio>
#include <deque>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "tensorflow/core/lib/db/sqlite.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/random/random.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"

// This experiment pre-allocates rows in batches with predicted sizes.
//
// step=1          size=12,288     ptime=0    itime=0          ctime=9,239      icnt=0       ucnt=0       dcnt=0
// step=5,001      size=360,448    ptime=58   itime=291,014    ctime=9,794      icnt=5,994   ucnt=5,000   dcnt=0
// step=10,001     size=655,360    ptime=62   itime=312,691    ctime=15,809     icnt=10,989  ucnt=10,000  dcnt=0
// step=15,001     size=958,464    ptime=74   itime=373,767    ctime=11,688     icnt=15,984  ucnt=15,000  dcnt=0
// step=20,001     size=1,261,568  ptime=125  itime=629,338    ctime=226,340    icnt=20,979  ucnt=20,000  dcnt=0
// step=25,001     size=1,568,768  ptime=113  itime=569,984    ctime=14,475     icnt=25,974  ucnt=25,000  dcnt=0
// step=30,001     size=1,867,776  ptime=62   itime=314,482    ctime=11,880     icnt=30,969  ucnt=30,000  dcnt=0
// step=35,001     size=2,179,072  ptime=53   itime=269,861    ctime=13,441     icnt=35,964  ucnt=35,000  dcnt=0
// step=40,001     size=2,498,560  ptime=58   itime=290,831    ctime=11,454     icnt=40,959  ucnt=40,000  dcnt=0
// step=45,001     size=2,822,144  ptime=58   itime=290,747    ctime=15,571     icnt=45,954  ucnt=45,000  dcnt=0
// final size = 3,137,536
// vacuum size = 2,719,744

#define ASSIGN_OR_RETURN_IF_ERROR(VAR, EXPR) \
  auto VAR##_so = EXPR;                      \
  TF_RETURN_IF_ERROR(VAR##_so.status());     \
  auto VAR = std::move(VAR##_so.ValueOrDie());

namespace tensorflow {
namespace {

const int64 kTagId = 123;

const char* kCreateTensorsTable = R"sql(
  CREATE TABLE IF NOT EXISTS Tensors (
    rowid INTEGER PRIMARY KEY,
    tag_id INTEGER NOT NULL,
    computed_time REAL,
    step INTEGER,
    dtype INTEGER,
    shape TEXT,
    data BLOB
  )
)sql";

const char* kCreateTensorIndex = R"sql(
  CREATE UNIQUE INDEX IF NOT EXISTS TensorIndex
  ON Tensors (tag_id, step)
  WHERE step IS NOT NULL
)sql";

const char* kPredictLength = R"sql(
  SELECT MAX(32, CAST(LENGTH(SNAP(?)) * 1.5 AS INTEGER))
)sql";

const char* kInsertTensor = R"sql(
  INSERT INTO Tensors (tag_id, data)
  VALUES (?, ZEROBLOB(?))
)sql";

const char* kUpdateTensor = R"sql(
  UPDATE
    Tensors
  SET
    computed_time = ?,
    step = ?,
    dtype = ?,
    shape = ?,
    data = SNAP(?)
  WHERE
    rowid = ?
)sql";

const char* kMoveTensor = R"sql(
  WITH T AS
  (
    SELECT
      computed_time,
      step,
      dtype,
      shape,
      data
    FROM
      Tensors
    WHERE
      rowid = ?
  )
  UPDATE
    Tensors
  SET
    computed_time = (SELECT computed_time FROM T),
    step = (SELECT step FROM T),
    dtype = (SELECT dtype FROM T),
    shape = (SELECT shape FROM T),
    data = (SELECT data FROM T)
  WHERE
    rowid = ?
)sql";

const char* kDeleteTensor = R"sql(
  DELETE FROM Tensors WHERE rowid = ?
)sql";

double GetWallTime(Env* env) {
  return static_cast<double>(env->NowMicros()) / 1.0e6;
}

string StringifyShape(const TensorShape& shape) {
  string result;
  bool first = true;
  for (const auto& dim : shape) {
    if (first) {
      first = false;
    } else {
      strings::StrAppend(&result, ",");
    }
    strings::StrAppend(&result, dim.size);
  }
  return result;
}

void BindTensorData(SqliteStatement* stmt, int parameter, const Tensor& t) {
  switch (t.dtype()) {
    case DT_STRING:
      stmt->BindBlobUnsafe(parameter, t.scalar<string>()());
      break;
    default:
      stmt->BindBlobUnsafe(parameter, t.tensor_data());
      break;
  }
}

struct Connection {
  std::shared_ptr<Sqlite> db;
  SqliteStatement vacuum;
  SqliteStatement ivacuum;
  SqliteStatement begin;
  SqliteStatement commit;
  SqliteStatement rollback;
  SqliteStatement predict_length;
  SqliteStatement insert_tensor;
  SqliteStatement update_tensor;
  SqliteStatement move_tensor;
  SqliteStatement delete_tensor;

  static xla::StatusOr<std::unique_ptr<Connection>> Open(
      const string& path, const string& auto_vacuum) {
    ASSIGN_OR_RETURN_IF_ERROR(db, Sqlite::Open(path));
    db->UseWriteAheadLogWithReducedDurabilityIfPossible();
    TF_RETURN_IF_ERROR(
        db->Prepare(strings::StrCat("PRAGMA auto_vacuum = ", auto_vacuum))
            .StepAndReset());
    TF_RETURN_IF_ERROR(db->Prepare(kCreateTensorsTable).StepAndReset());
    TF_RETURN_IF_ERROR(db->Prepare(kCreateTensorIndex).StepAndReset());
    return std::unique_ptr<Connection>(new Connection(std::move(db)));
  }

  Status Close() {
    Status s;
    s.Update(vacuum.Close());
    s.Update(ivacuum.Close());
    s.Update(begin.Close());
    s.Update(commit.Close());
    s.Update(rollback.Close());
    s.Update(predict_length.Close());
    s.Update(insert_tensor.Close());
    s.Update(update_tensor.Close());
    s.Update(move_tensor.Close());
    s.Update(delete_tensor.Close());
    s.Update(db->Close());
    return s;
  }

 private:
  explicit Connection(std::shared_ptr<Sqlite> ptr)
      : db(std::move(ptr)),
        vacuum(db->Prepare("VACUUM")),
        ivacuum(db->Prepare("PRAGMA incremental_vacuum")),
        begin(db->Prepare("BEGIN TRANSACTION")),
        commit(db->Prepare("COMMIT")),
        rollback(db->Prepare("ROLLBACK")),
        predict_length(db->Prepare(kPredictLength)),
        insert_tensor(db->Prepare(kInsertTensor)),
        update_tensor(db->Prepare(kUpdateTensor)),
        move_tensor(db->Prepare(kMoveTensor)),
        delete_tensor(db->Prepare(kDeleteTensor)) {}
};

int main(int argc, char* argv[]) {
  string path = "/tmp/experiment.sqlite";
  string auto_vacuum = "NONE";
  int64 slots = 1000;
  int64 inserts = 50000;
  int64 report_interval = 5000;
  std::vector<Flag> flag_list = {
      Flag("db", &path, "Path of SQLite DB file"),
      Flag("auto_vacuum", &auto_vacuum, "Can be NONE, FULL, or INCREMENTAL"),
      Flag("slots", &slots, "Target length for series"),
      Flag("inserts", &inserts, "Number of items to insert"),
      Flag("report_interval", &report_interval, "Steps between reports"),
  };
  string usage = Flags::Usage(argv[0], flag_list);
  const bool parseResult = Flags::Parse(&argc, argv, flag_list);
  if (!parseResult || path.empty()) {
    std::cerr << usage;
    return -1;
  }
  port::InitMain(argv[0], &argc, &argv);
  setlocale(LC_NUMERIC, "");
  Env* env = Env::Default();
  if (env->FileExists(path).ok()) {
    TF_CHECK_OK(env->DeleteFile(path));
  }

  auto conn = Connection::Open(path, auto_vacuum).ValueOrDie();

//  Tensor t{DT_INT64, {}};
//  t.scalar<int64>()() = 14;

//  Tensor t{DT_INT64, {100}};
//  for (int64 i = 0; i < 100; ++i) {
//    t.flat<int64>()(i) = i;
//  }

  Tensor t{DT_STRING, {}};
  t.scalar<string>()() = "hello world";

  uint64 size;
  uint64 start = env->NowMicros();
  int icnt = 0;
  int dcnt = 0;
  int ucnt = 0;
  std::deque<int64> rowids;

  for (int64 step = 1; step < inserts; ++step) {
    if (step % report_interval == 1) {
      uint64 itime = env->NowMicros() - start;
      start = env->NowMicros();
      TF_CHECK_OK(conn->Close());
      uint64 ctime = env->NowMicros() - start;
      TF_CHECK_OK(env->GetFileSize(path, &size));
      printf(
          "step=%'-10llu size=%'-10llu ptime=%'-4llu itime=%'-10llu "
              "ctime=%'-10llu icnt=%'-7d ucnt=%'-7d dcnt=%'-7d\n",
          step, size, itime / report_interval, itime, ctime, icnt, ucnt, dcnt);
      fflush(stdout);
      conn = Connection::Open(path, auto_vacuum).ValueOrDie();
      start = env->NowMicros();
    }

    if (rowids.empty()) {
      // TF_CHECK_OK(conn->ivacuum.StepAndReset());
      TF_CHECK_OK(conn->begin.StepAndReset());
      BindTensorData(&conn->predict_length, 1, t);
      TF_CHECK_OK(conn->predict_length.Step());
      for (int64 i = 1; i < slots; ++i) {
        conn->insert_tensor.BindInt(1, kTagId);
        conn->insert_tensor.BindInt(2, conn->predict_length.ColumnInt(0));
        TF_CHECK_OK(conn->insert_tensor.StepAndReset());
        rowids.push_back(conn->db->LastInsertRowid());
        ++icnt;
      }
      conn->predict_length.Reset();
      TF_CHECK_OK(conn->commit.StepAndReset());
    }

    conn->update_tensor.BindDouble(1, GetWallTime(env));
    conn->update_tensor.BindInt(2, step);
    conn->update_tensor.BindInt(3, t.dtype());
    if (!TensorShapeUtils::IsScalar(t.shape())) {
      conn->update_tensor.BindText(4, StringifyShape(t.shape()));
    }
    BindTensorData(&conn->update_tensor, 5, t);
    conn->update_tensor.BindInt(6, rowids.front());
    TF_CHECK_OK(conn->update_tensor.StepAndReset());
    rowids.pop_front();
    ++ucnt;
  }

  TF_CHECK_OK(conn->Close());
  TF_CHECK_OK(env->GetFileSize(path, &size));
  printf("final size = %'llu\n", size);
  conn = Connection::Open(path, auto_vacuum).ValueOrDie();
  TF_CHECK_OK(conn->vacuum.StepAndReset());
  TF_CHECK_OK(conn->Close());
  TF_CHECK_OK(env->GetFileSize(path, &size));
  printf("vacuum size = %'llu\n", size);

  return 0;
}

}  // namespace
}  // namespace tensorflow

int main(int argc, char* argv[]) { return tensorflow::main(argc, argv); }

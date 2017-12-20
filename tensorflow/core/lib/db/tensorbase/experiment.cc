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

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/db/sqlite.h"
#include "tensorflow/core/lib/random/random.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"

namespace tensorflow {
namespace {

const int64 kTagId = 123;

const char* kCreateTensorsTable = R"sql(
  CREATE TABLE IF NOT EXISTS Tensors (
    rowid INTEGER PRIMARY KEY,
    tag_id INTEGER NOT NULL,
    step INTEGER,
    computed_time REAL,
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
    step = ?,
    computed_time = ?,
    dtype = ?,
    shape = ?,
    data = SNAP(?)
  WHERE
    rowid = ?
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
  SqliteStatement begin;
  SqliteStatement commit;
  SqliteStatement rollback;
  SqliteStatement predict_length;
  SqliteStatement insert_tensor;
  SqliteStatement update_tensor;

  static xla::StatusOr<std::unique_ptr<Connection>> Open(
      const string& path, const string& auto_vacuum) {
    auto db = Sqlite::Open(path).ValueOrDie();
    db->TryToSetPageSize(4096);
    TF_CHECK_OK(db->MakeSqliteGoodForWriteHeavyUseCases());
    TF_CHECK_OK(db->SetBusyTimeout(10 * 1000));
    TF_RETURN_IF_ERROR(
        db->Prepare(strings::StrCat("PRAGMA auto_vacuum = ", auto_vacuum))
            .ValueOrDie()
            .StepAndReset());
    TF_RETURN_IF_ERROR(
        db->Prepare(kCreateTensorsTable).ValueOrDie().StepAndReset());
    TF_RETURN_IF_ERROR(
        db->Prepare(kCreateTensorIndex).ValueOrDie().StepAndReset());
    return std::unique_ptr<Connection>(new Connection(std::move(db)));
  }

 private:
  explicit Connection(std::shared_ptr<Sqlite> ptr)
      : db(std::move(ptr)),
        vacuum(std::move(db->Prepare("VACUUM").ValueOrDie())),
        begin(std::move(db->Prepare("BEGIN TRANSACTION").ValueOrDie())),
        commit(std::move(db->Prepare("COMMIT").ValueOrDie())),
        rollback(std::move(db->Prepare("ROLLBACK").ValueOrDie())),
        predict_length(std::move(db->Prepare(kPredictLength).ValueOrDie())),
        insert_tensor(std::move(db->Prepare(kInsertTensor).ValueOrDie())),
        update_tensor(std::move(db->Prepare(kUpdateTensor).ValueOrDie())) {}
};

int main(int argc, char* argv[]) {
  string path = "/tmp/experiment.sqlite";
  string auto_vacuum = "NONE";
  int64 slots = 1000;
  int64 inserts = 5000000;
  int64 report_interval = 500;
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

  Tensor t{DT_INT64, {}};
  t.scalar<int64>()() = 14;

  // Tensor t{DT_INT64, {100}};
  // for (int64 i = 0; i < 100; ++i) {
  //   t.flat<int64>()(i) = i;
  // }

  //  Tensor t{DT_STRING, {}};
  //  t.scalar<string>()() = "hello world";

  uint64 size;
  uint64 start = env->NowMicros();
  int icnt = 0;
  int mcnt = 0;
  int ucnt = 0;
  std::vector<int64> rowids;

  TF_CHECK_OK(conn->begin.StepAndReset());
  BindTensorData(&conn->predict_length, 1, t);
  TF_CHECK_OK(conn->predict_length.Step());
  for (int64 i = 0; i < slots + 1; ++i) {
    conn->insert_tensor.BindInt(1, kTagId);
    conn->insert_tensor.BindInt(2, conn->predict_length.ColumnInt(0));
    TF_CHECK_OK(conn->insert_tensor.StepAndReset());
    rowids.push_back(conn->db->last_insert_row_id());
    ++icnt;
  }
  conn->predict_length.Reset();
  TF_CHECK_OK(conn->commit.StepAndReset());

  for (int64 i = 0; i < inserts; ++i) {
    if (i % report_interval == 1) {
      uint64 itime = env->NowMicros() - start;
      start = env->NowMicros();
      conn.reset();
      uint64 ctime = env->NowMicros() - start;
      TF_CHECK_OK(env->GetFileSize(path, &size));
      printf(
          "step=%'-10llu size=%'-10llu ptime=%'-4llu itime=%'-10llu "
          "ctime=%'-10llu icnt=%'-7d ucnt=%'-7d mcnt=%'-7d\n",
          i + 1, size, itime / report_interval, itime, ctime, icnt, ucnt, mcnt);
      fflush(stdout);
      conn = Connection::Open(path, auto_vacuum).ValueOrDie();
      start = env->NowMicros();
    }

    int64 rowid;
    if (i < slots) {
      rowid = rowids[i];
    } else {
      int64 j = static_cast<int64>(random::New64() % static_cast<uint64>(i));
      if (j < slots) {
        rowid = rowids[j];
      } else {
        rowid = rowids[slots];
      }
    }

    conn->update_tensor.BindDouble(1, GetWallTime(env));
    conn->update_tensor.BindInt(2, i + 1);
    conn->update_tensor.BindInt(3, t.dtype());
    if (!TensorShapeUtils::IsScalar(t.shape())) {
      conn->update_tensor.BindText(4, StringifyShape(t.shape()));
    }
    BindTensorData(&conn->update_tensor, 5, t);
    conn->update_tensor.BindInt(6, rowid);
    TF_CHECK_OK(conn->update_tensor.StepAndReset());
    ++ucnt;
  }

  conn.reset();
  TF_CHECK_OK(env->GetFileSize(path, &size));
  printf("final size = %'llu\n", size);
  conn = Connection::Open(path, auto_vacuum).ValueOrDie();
  TF_CHECK_OK(conn->vacuum.StepAndReset());
  conn.reset();
  TF_CHECK_OK(env->GetFileSize(path, &size));
  printf("vacuum size = %'llu\n", size);

  return 0;
}

}  // namespace
}  // namespace tensorflow

int main(int argc, char* argv[]) { return tensorflow::main(argc, argv); }

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

// Here are the results of this experiment:
//
// step=1          size=12,288     ptime=0    itime=1          ctime=5,002      icnt=0       dcnt=0
// step=500,001    size=102,400    ptime=1    itime=858,765    ctime=15,531     icnt=7,166   dcnt=6,166
// step=1,000,001  size=102,400    ptime=0    itime=140,684    ctime=15,296     icnt=7,861   dcnt=6,861
// step=1,500,001  size=102,400    ptime=0    itime=66,813     ctime=22,513     icnt=8,267   dcnt=7,267
// step=2,000,001  size=102,400    ptime=0    itime=73,359     ctime=7,450      icnt=8,546   dcnt=7,546
// step=2,500,001  size=102,400    ptime=0    itime=30,194     ctime=17,474     icnt=8,745   dcnt=7,745
// step=3,000,001  size=102,400    ptime=0    itime=30,693     ctime=13,527     icnt=8,921   dcnt=7,921
// step=3,500,001  size=102,400    ptime=0    itime=44,988     ctime=11,136     icnt=9,073   dcnt=8,073
// step=4,000,001  size=102,400    ptime=0    itime=40,837     ctime=12,160     icnt=9,224   dcnt=8,224
// step=4,500,001  size=102,400    ptime=0    itime=29,355     ctime=9,957      icnt=9,330   dcnt=8,330
// final size = 102,400
// vacuum size = 73,728

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
)sql";

const char* kInsertTensor = R"sql(
  INSERT INTO Tensors (tag_id, computed_time, step, dtype, shape, data)
  VALUES (?, ?, ?, ?, ?, snap(?))
)sql";

const char* kDeleteTensor = R"sql(
  DELETE FROM Tensors WHERE rowid = ?
)sql";

const char* kVacuum = R"sql(
  VACUUM
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

struct Connection {
  std::shared_ptr<Sqlite> db;
  SqliteStatement vacuum;
  SqliteStatement insert_tensor;
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
    s.Update(insert_tensor.Close());
    s.Update(delete_tensor.Close());
    s.Update(db->Close());
    return s;
  }

 private:
  explicit Connection(std::shared_ptr<Sqlite> ptr)
      : db(std::move(ptr)),
        vacuum(db->Prepare(kVacuum)),
        insert_tensor(db->Prepare(kInsertTensor)),
        delete_tensor(db->Prepare(kDeleteTensor)) {}
};

int main(int argc, char* argv[]) {
  string path = "/tmp/experiment.sqlite";
  string auto_vacuum = "NONE";
  int64 slots = 1000;
  int64 inserts = 5000000;
  int64 report_interval = 500000;
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

  uint64 size;
  uint64 start = env->NowMicros();
  int icnt = 0;
  int dcnt = 0;
  std::vector<int64> rowids;
  for (int64 step = 1; step < inserts; ++step) {
    if (step % report_interval == 1) {
      uint64 itime = env->NowMicros() - start;
      start = env->NowMicros();
      TF_CHECK_OK(conn->Close());
      uint64 ctime = env->NowMicros() - start;
      TF_CHECK_OK(env->GetFileSize(path, &size));
      printf(
          "step=%'-10llu size=%'-10llu ptime=%'-4llu itime=%'-10llu "
              "ctime=%'-10llu icnt=%'-7d dcnt=%'-7d\n",
          step, size, itime / report_interval, itime, ctime, icnt, dcnt);
      fflush(stdout);
      conn = Connection::Open(path, auto_vacuum).ValueOrDie();
      start = env->NowMicros();
    }
    if (rowids.size() == slots) {
      int64 j = static_cast<int64>(random::New64() % static_cast<uint64>(step));
      if (j >= slots) continue;
      conn->delete_tensor.BindInt(1, rowids[j]);
      TF_CHECK_OK(conn->delete_tensor.StepAndReset());
      rowids.erase(rowids.begin() + j);
      ++dcnt;
    }
    conn->insert_tensor.BindInt(1, kTagId);
    conn->insert_tensor.BindDouble(2, GetWallTime(env));
    conn->insert_tensor.BindInt(3, step);
    conn->insert_tensor.BindInt(4, t.dtype());
    if (!TensorShapeUtils::IsScalar(t.shape())) {
      conn->insert_tensor.BindText(5, StringifyShape(t.shape()));
    }
    conn->insert_tensor.BindBlobUnsafe(6, t.tensor_data());
    TF_CHECK_OK(conn->insert_tensor.StepAndReset());
    rowids.push_back(conn->db->LastInsertRowid());
    ++icnt;
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

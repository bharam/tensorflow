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

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/db/sqlite.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"

// Here are the results of this experiment:
//
// step=1        size=12,288     ptime=0    itime=46         ctime=11,093
// step=5,001    size=266,240    ptime=58   itime=291,097    ctime=10,291
// step=10,001   size=512,000    ptime=59   itime=295,597    ctime=11,304
// step=15,001   size=761,856    ptime=62   itime=313,355    ctime=14,515
// step=20,001   size=1,015,808  ptime=57   itime=289,037    ctime=13,015
// step=25,001   size=1,273,856  ptime=59   itime=299,922    ctime=14,073
// step=30,001   size=1,523,712  ptime=206  itime=1,030,990  ctime=15,561
// step=35,001   size=1,785,856  ptime=56   itime=282,692    ctime=10,974
// step=40,001   size=2,060,288  ptime=65   itime=329,116    ctime=13,634
// step=45,001   size=2,330,624  ptime=61   itime=306,550    ctime=11,796
// final size = 2,600,960
// vacuum size = 2,510,848

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
  DELETE FROM Tensors WHERE tag_id = ? AND step = ?
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

class Connection {
 public:
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
    s.Update(db_->Close());
    return s;
  }

 private:
  explicit Connection(std::shared_ptr<Sqlite> db)
      : db_(std::move(db)),
        vacuum(db_->Prepare(kVacuum)),
        insert_tensor(db_->Prepare(kInsertTensor)),
        delete_tensor(db_->Prepare(kDeleteTensor)) {}

  std::shared_ptr<Sqlite> db_;

 public:
  SqliteStatement vacuum;
  SqliteStatement insert_tensor;
  SqliteStatement delete_tensor;
};

int main(int argc, char* argv[]) {
  string path = "/tmp/experiment.sqlite";
  string auto_vacuum = "NONE";
  int64 slots = 100;
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

  Tensor t{DT_INT64, {}};
  t.scalar<int64>()() = 14;

  uint64 size;
  uint64 start = env->NowMicros();
  for (int64 step = 1; step < inserts; ++step) {
    conn->insert_tensor.BindInt(1, kTagId);
    conn->insert_tensor.BindDouble(2, GetWallTime(env));
    conn->insert_tensor.BindInt(3, step);
    conn->insert_tensor.BindInt(4, t.dtype());
    if (!TensorShapeUtils::IsScalar(t.shape())) {
      conn->insert_tensor.BindText(5, StringifyShape(t.shape()));
    }
    conn->insert_tensor.BindBlobUnsafe(6, t.tensor_data());
    TF_CHECK_OK(conn->insert_tensor.StepAndReset());
    if (step % report_interval == 1) {
      uint64 itime = env->NowMicros() - start;
      start = env->NowMicros();
      TF_CHECK_OK(conn->Close());
      uint64 ctime = env->NowMicros() - start;
      TF_CHECK_OK(env->GetFileSize(path, &size));
      printf(
          "step=%'-8llu size=%'-10llu ptime=%'-4llu itime=%'-10llu "
              "ctime=%'-10llu\n",
          step, size, itime / report_interval, itime, ctime);
      fflush(stdout);
      conn = Connection::Open(path, auto_vacuum).ValueOrDie();
      start = env->NowMicros();
    }
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

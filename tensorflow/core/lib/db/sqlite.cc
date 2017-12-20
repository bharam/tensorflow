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
#include "tensorflow/core/lib/db/sqlite.h"

extern "C" int sqlite3_snapfn_init(sqlite3*, const char**, const void*);

namespace tensorflow {

/* static */
template <typename... Args>
Status Sqlite::MakeStatus(int code, Args&&... args) {
  // See: https://sqlite.org/rescode.html
  switch (code & 0xff) {
    case SQLITE_OK:
    case SQLITE_ROW:
    case SQLITE_DONE:
      return Status::OK();
    case SQLITE_ABORT:  // Callback routine requested an abort
      return errors::Aborted(std::forward<Args>(args)...);
    case SQLITE_READONLY:  // Attempt to write a readonly database
    case SQLITE_MISMATCH:  // Data type mismatch
      return errors::FailedPrecondition(std::forward<Args>(args)...);
    case SQLITE_MISUSE:    // Library used incorrectly
    case SQLITE_INTERNAL:  // Internal logic error in SQLite
      return errors::Internal(std::forward<Args>(args)...);
    case SQLITE_RANGE:  // 2nd parameter to sqlite3_bind out of range
      return errors::OutOfRange(std::forward<Args>(args)...);
    case SQLITE_CANTOPEN:    // Unable to open the database file
    case SQLITE_CONSTRAINT:  // Abort due to constraint violation
    case SQLITE_NOTFOUND:    // Unknown opcode or statement parameter name
    case SQLITE_NOTADB:      // File opened that is not a database file
      return errors::InvalidArgument(std::forward<Args>(args)...);
    case SQLITE_CORRUPT:  // The database disk image is malformed
      return errors::DataLoss(std::forward<Args>(args)...);
    case SQLITE_AUTH:  // Authorization denied
    case SQLITE_PERM:  // Access permission denied
      return errors::PermissionDenied(std::forward<Args>(args)...);
    case SQLITE_FULL:    // Insertion failed because database is full
    case SQLITE_TOOBIG:  // String or BLOB exceeds size limit
    case SQLITE_NOLFS:   // Uses OS features not supported on host
      return errors::ResourceExhausted(std::forward<Args>(args)...);
    case SQLITE_BUSY:      // The database file is locked
    case SQLITE_LOCKED:    // A table in the database is locked
    case SQLITE_PROTOCOL:  // Database lock protocol error
    case SQLITE_NOMEM:     // A malloc() failed
      return errors::Unavailable(std::forward<Args>(args)...);
    case SQLITE_INTERRUPT:  // Operation terminated by sqlite3_interrupt
      return errors::Cancelled(std::forward<Args>(args)...);
    case SQLITE_ERROR:   // SQL error or missing database
    case SQLITE_IOERR:   // Some kind of disk I/O error occurred
    case SQLITE_SCHEMA:  // The database schema changed
    default:
      return errors::Unknown(std::forward<Args>(args)...);
  }
}

/* static */
xla::StatusOr<std::shared_ptr<Sqlite>> Sqlite::Open(string path, int flags) {
  sqlite3* sqlite = nullptr;
  int rc = sqlite3_open_v2(path.c_str(), &sqlite, flags, nullptr);
  if (rc != SQLITE_OK) {
    return MakeStatus(rc, sqlite3_errstr(rc), ": ", path);
  }
  CHECK_EQ(SQLITE_OK, sqlite3_snapfn_init(sqlite, nullptr, nullptr));
  auto result = std::shared_ptr<Sqlite>(new Sqlite(sqlite, std::move(path)));
  result->self_ = std::weak_ptr<Sqlite>(result);
  return result;
}

xla::StatusOr<SqliteStatement> Sqlite::Prepare(const string& sql) {
  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, sql.c_str(), sql.size() + 1, &stmt, nullptr);
  if (TF_PREDICT_TRUE(rc == SQLITE_OK)) {
    SqliteStatement result{stmt, self_.lock()};
    return result;
  }
  return MakeStatus(rc, "Prepare failed: ", sqlite3_errmsg(db_), " for ", sql);
}

Status SqliteStatement::Step(bool* isDone) {
  if (TF_PREDICT_FALSE(stmt_ == nullptr)) {
    *isDone = true;
    return errors::FailedPrecondition("sqlite3_stmt absent");
  }
  if (TF_PREDICT_FALSE(bind_error_ != SQLITE_OK)) {
    *isDone = true;
    return Sqlite::MakeStatus(bind_error_, db_->errmsg(), " on parameter ",
                              bind_error_parameter_, " for ",
                              sqlite3_sql(stmt_));
  }
  int rc = sqlite3_step(stmt_);
  switch (rc) {
    case SQLITE_ROW:
      *isDone = false;
      return Status::OK();
    case SQLITE_DONE:
      *isDone = true;
      return Status::OK();
    default:
      *isDone = true;
      return Sqlite::MakeStatus(rc, "Step failed: ", db_->errmsg(), " for ",
                                sqlite3_sql(stmt_));
  }
}

}  // namespace tensorflow

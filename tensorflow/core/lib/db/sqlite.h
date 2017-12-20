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
#ifndef TENSORFLOW_CORE_LIB_DB_SQLITE_H_
#define TENSORFLOW_CORE_LIB_DB_SQLITE_H_

#include <cstddef>
#include <memory>
#include <utility>

#include "sqlite3.h"
#include "tensorflow/compiler/xla/statusor.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/platform/types.h"

namespace tensorflow {

class Sqlite;

/// \brief SQLite prepared statement.
///
/// Instances of this class, along with its associated Sqlite
/// connection object, can be shared between multiple threads, but the
/// caller is respnosible for using a mutex to serialize access so only
/// a single thread accesses them all at a given moment.
class SqliteStatement {
 public:
  SqliteStatement() noexcept = default;
  ~SqliteStatement() { sqlite3_finalize(stmt_); }

  /// \brief Move constructor, after which <other> should not be used.
  SqliteStatement(SqliteStatement&& other) noexcept
      : stmt_(other.stmt_),
        bind_error_(other.bind_error_),
        db_(std::move(other.db_)) {
    other.stmt_ = nullptr;
    other.bind_error_ = SQLITE_OK;
  }

  /// \brief Move assignment, after which <other> should not be used.
  SqliteStatement& operator=(SqliteStatement&& other) noexcept {
    if (&other != this) {
      sqlite3_finalize(stmt_);
      stmt_ = other.stmt_;
      bind_error_ = other.bind_error_;
      db_ = std::move(other.db_);
      other.stmt_ = nullptr;
      other.bind_error_ = SQLITE_OK;
    }
    return *this;
  }

  /// \brief Returns true if statement is not empty.
  explicit operator bool() const { return stmt_ != nullptr; }

  /// \brief Executes query for fetching arbitrary rows.
  ///
  /// `isDone` will always be set to true unless SQLITE_ROW is returned
  /// by the underlying API. If status() is already in an error state,
  /// then this method is a no-op and the existing status is returned.
  Status Step(bool* is_done);

  /// \brief Executes query when one row is desired.
  Status Step() {
    bool is_done;
    TF_RETURN_IF_ERROR(Step(&is_done));
    if (TF_PREDICT_FALSE(is_done)) {
      return errors::Internal("wanted sqlite row: ", sqlite3_sql(stmt_));
    }
    return Status::OK();
  }

  /// \brief Executes query when no rows must be returned and Reset().
  Status StepAndReset() {
    bool is_done;
    Status s = Step(&is_done);
    if (TF_PREDICT_TRUE(s.ok()) && TF_PREDICT_FALSE(!is_done)) {
      s = errors::Internal("unexpected sqlite row: ", sqlite3_sql(stmt_));
    }
    Reset();
    return s;
  }

  /// \brief Resets statement so it can be executed again.
  ///
  /// - Resets the prepared statement
  /// - Sets all Bind*() values to NULL
  void Reset() {
    if (TF_PREDICT_TRUE(stmt_ != nullptr)) {
      sqlite3_reset(stmt_);
      sqlite3_clear_bindings(stmt_);  // not nullptr friendly
    }
    bind_error_ = SQLITE_OK;
  }

  /// \brief Binds signed 64-bit integer to 1-indexed query parameter.
  void BindInt(int parameter, int64 value) {
    Update(sqlite3_bind_int64(stmt_, parameter, value), parameter);
  }
  void BindInt(const char* parameter, int64 value) {
    BindInt(GetParameterIndex(parameter), value);
  }

  /// \brief Binds double to 1-indexed query parameter.
  void BindDouble(int parameter, double value) {
    Update(sqlite3_bind_double(stmt_, parameter, value), parameter);
  }
  void BindDouble(const char* parameter, double value) {
    BindDouble(GetParameterIndex(parameter), value);
  }

  /// \brief Copies UTF-8 text to 1-indexed query parameter.
  ///
  /// If NUL characters are present, they will still go in the DB and
  /// be successfully retrieved by ColumnString(); however, the
  /// behavior of these values with SQLite functions is undefined.
  void BindText(int parameter, const StringPiece& text) {
    Update(sqlite3_bind_text64(stmt_, parameter, text.data(), text.size(),
                               SQLITE_TRANSIENT, SQLITE_UTF8),
           parameter);
  }
  void BindText(const char* parameter, const StringPiece& text) {
    BindText(GetParameterIndex(parameter), text);
  }

  /// \brief Copies binary data to 1-indexed query parameter.
  void BindBlob(int parameter, const StringPiece& blob) {
    Update(sqlite3_bind_blob64(stmt_, parameter, blob.data(), blob.size(),
                               SQLITE_TRANSIENT),
           parameter);
  }
  void BindBlob(const char* parameter, const StringPiece& blob) {
    BindBlob(GetParameterIndex(parameter), blob);
  }

  /// \brief Binds UTF-8 text to 1-indexed query parameter.
  ///
  /// The contents of `text` must not be changed or freed until Reset()
  /// or Close() is called.
  ///
  /// If NUL characters are present, they will still go in the DB and
  /// be successfully retrieved by ColumnString(); however, the
  /// behavior of these values with SQLite functions is undefined.
  void BindTextUnsafe(int parameter, const StringPiece& text) {
    Update(sqlite3_bind_text64(stmt_, parameter, text.data(), text.size(),
                               SQLITE_STATIC, SQLITE_UTF8),
           parameter);
  }
  void BindTextUnsafe(const char* parameter, const StringPiece& text) {
    BindTextUnsafe(GetParameterIndex(parameter), text);
  }

  /// \brief Binds binary data to 1-indexed query parameter.
  ///
  /// The contents of `blob` must not be changed or freed until Reset()
  /// or Close() is called.
  void BindBlobUnsafe(int parameter, const StringPiece& blob) {
    Update(sqlite3_bind_blob64(stmt_, parameter, blob.data(), blob.size(),
                               SQLITE_STATIC),
           parameter);
  }
  void BindBlobUnsafe(const char* parameter, const StringPiece& text) {
    BindBlobUnsafe(GetParameterIndex(parameter), text);
  }

  /// \brief Creates an empty placeholder for a BLOB written later.
  void BindZeroBlob(int parameter, uint64 size) {
    Update(sqlite3_bind_zeroblob64(stmt_, parameter, size), parameter);
  }
  void BindZeroBlob(const char* parameter, uint64 size) {
    BindZeroBlob(GetParameterIndex(parameter), size);
  }

  /// \brief Returns number of columns in result set.
  int ColumnCount() TF_MUST_USE_RESULT { return sqlite3_column_count(stmt_); }

  /// \brief Returns type of 0-indexed column value in row data.
  ///
  /// Please note that SQLite is dynamically typed and the type of a
  /// particular column can vary from row to row.
  int ColumnType(int column) TF_MUST_USE_RESULT {
    return sqlite3_column_type(stmt_, column);
  }

  /// \brief Returns 0-indexed column from row result coerced as an integer.
  int64 ColumnInt(int column) TF_MUST_USE_RESULT {
    return sqlite3_column_int64(stmt_, column);
  }

  /// \brief Returns 0-indexed column from row result coerced as a double.
  double ColumnDouble(int column) TF_MUST_USE_RESULT {
    return sqlite3_column_double(stmt_, column);
  }

  /// \brief Copies 0-indexed column from row result coerced as a string.
  ///
  /// NULL values are returned as empty string. This method should be
  /// used for both BLOB and TEXT columns. See also: ColumnType().
  string ColumnString(int column) TF_MUST_USE_RESULT {
    auto data = sqlite3_column_blob(stmt_, column);
    if (data == nullptr) {
      return "";
    }
    return {static_cast<const char*>(data),
            static_cast<size_t>(ColumnSize(column))};
  }

  /// \brief Returns pointer to binary data at 0-indexed column.
  ///
  /// The returned memory will be mutated or freed the next time
  /// Step() or Reset() is called. No NUL terminator is added. See
  /// ColumnSize(). Please note that an empty BLOB is NULL.
  const char* ColumnStringUnsafe(int column) TF_MUST_USE_RESULT {
    return static_cast<const char*>(sqlite3_column_blob(stmt_, column));
  }

  /// \brief Returns number of bytes stored at 0-indexed column.
  int ColumnSize(int column) TF_MUST_USE_RESULT {
    return sqlite3_column_bytes(stmt_, column);
  }

 private:
  friend Sqlite;
  SqliteStatement(sqlite3_stmt* stmt, std::shared_ptr<Sqlite> db) noexcept
      : stmt_(stmt), db_(std::move(db)) {}

  void Update(int rc, int parameter) {
    if (TF_PREDICT_FALSE(rc != SQLITE_OK)) {
      if (bind_error_ == SQLITE_OK) {
        bind_error_ = rc;
        bind_error_parameter_ = parameter;
      }
    }
  }

  int GetParameterIndex(const char* parameter) {
    // Each call to this function requires O(n) strncmp().
    int index = sqlite3_bind_parameter_index(stmt_, parameter);
    if (TF_PREDICT_FALSE(index == 0)) {
      bind_error_ = SQLITE_NOTFOUND;
      bind_error_parameter_ = 0;
    }
    return index;
  }

  sqlite3_stmt* stmt_ = nullptr;
  int bind_error_ = SQLITE_OK;
  int bind_error_parameter_ = 0;
  std::shared_ptr<Sqlite> db_;

  TF_DISALLOW_COPY_AND_ASSIGN(SqliteStatement);
};

/// \brief SQLite connection object.
///
/// This class is a thin wrapper around `sqlite3` that makes it easier
/// and safer to use SQLite in the TensorFlow C++ codebase. It removes
/// deprecated APIs, improves the safety of others, adds helpers, and
/// pretends UTF16 doesn't exist.
///
/// Instances of this class and its associated statements must be used
/// by only a single thread at a time. If this pointer is shared across
/// threads, then the caller is responsible for having a mutex to
/// serialize access appropriately.
///
/// Destructors finalize statements and close the connection in the
/// correct order, using smart pointers.
class Sqlite {
 public:
  ~Sqlite() { CHECK_EQ(SQLITE_OK, sqlite3_close(db_)); }

  /// \brief Opens SQLite database file.
  ///
  /// See https://sqlite.org/c3ref/open.html
  static xla::StatusOr<std::shared_ptr<Sqlite>> Open(string path, int flags);

  /// \brief Opens SQLite DB in read/write/create mode.
  static xla::StatusOr<std::shared_ptr<Sqlite>> Open(string path) {
    return Open(std::move(path), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
  }

  /// \brief Tries to set page size on database.
  ///
  /// This is really chosen once when the database is created, but we
  /// can still try to change it; and it's totally worthwhile since
  /// we're storing blobs. This was 1024 by default for the longest
  /// time, until 3.12.0 (c. 2016) when it went up to 4096, which is
  /// much more reasonable. However some folks might want to set it
  /// even higher.
  ///
  /// For example, the SQLite authors did some measurements and they
  /// determinehd that a page size of 8192 is really the optimal for
  /// for BLOB: https://www.sqlite.org/intern-v-extern-blob.html
  ///
  /// The return value is what the page size actually is, after our
  /// attempt at change.
  int TryToSetPageSize(int bytes) {
    auto stmt =
        Prepare(strings::StrCat("PRAGMA page_size=", bytes)).ValueOrDie();
    TF_CHECK_OK(stmt.StepAndReset());
    stmt = Prepare("PRAGMA page_size").ValueOrDie();
    TF_CHECK_OK(stmt.Step());
    return static_cast<int>(stmt.ColumnInt(0));
  }

  /// \brief Sets busy timeout in milliseconds if possible.
  ///
  /// The default behavior is stuff just fails if the database is busy,
  /// so it's strongly recommended to call this method. Doing so causes
  /// the SQLite API to do exponential back-off retries whenever it it
  /// encounters SQLITE_BUSY result. This can be very common if more
  /// than one process is accessing a database file.
  Status SetBusyTimeout(int ms) {
    int rc = sqlite3_busy_timeout(db_, ms);
    if (rc != SQLITE_OK) return MakeStatus(rc, sqlite3_errmsg(db_));
    return Status::OK();
  }

  /// \brief Enables WAL mode and sets synchronous to NORMAL.
  ///
  /// Take for example TensorFlow summaries. If this method is called
  /// after opening the connection, they'll get written without needing
  /// transactions 4µs rather than 4ms! SQLite then uses shared memory
  /// to make those writes immediately available to other processes,
  /// e.g. TensorBoard.
  ///
  /// Much of that performance is gained by avoiding many calls to
  /// fsync() and fdatasync(). This is actually a reasonable thing to
  /// to do in WAL mode. The SQLite authors assure us that while a
  /// power loss might result in us losing a few writes, the database
  /// itself can't become corrupted.
  ///
  /// If the connection is in :memory: mode then this returns OK.
  Status MakeSqliteGoodForWriteHeavyUseCases() {
    auto stmt = Prepare("PRAGMA journal_mode=wal").ValueOrDie();
    TF_RETURN_IF_ERROR(stmt.Step());
    if (stmt.ColumnString(0) != "memory") return Status::OK();
    if (stmt.ColumnString(0) != "wal") {
      return errors::FailedPrecondition(
          "The SQLite journal_mode of ", path_, " is '", stmt.ColumnString(0),
          "' but we need it to be able to set it to 'wal' for write-intensive "
          "purposes; perhaps provide a fresh database file");
    }
    return Prepare("PRAGMA synchronous=NORMAL").ValueOrDie().StepAndReset();
  }

  /// \brief Creates SQLite statement.
  xla::StatusOr<SqliteStatement> Prepare(const string& sql);

  /// \brief Returns rowid assigned to last successful insert.
  int64 last_insert_row_id() { return sqlite3_last_insert_rowid(db_); }

  /// \brief Returns pointer to current error message state.
  const char* errmsg() { return sqlite3_errmsg(db_); }

  /// \brief Returns primary result code of last error.
  ///
  /// If the most recent API call was successful, the result is
  /// undefined. This should be the same as extended_errcode() & 0xff.
  int errcode() { return sqlite3_errcode(db_); }

  /// \brief Returns extended error code of last error.
  ///
  /// If the most recent API call was successful, the result is
  /// undefined.
  int extended_errcode() { return sqlite3_extended_errcode(db_); }

 private:
  friend SqliteStatement;
  Sqlite(sqlite3* db, const string path) noexcept
      : db_(db), path_(std::move(path)) {}
  template <typename... Args>
  static Status MakeStatus(int code, Args&&... args);

  sqlite3* db_;
  const string path_;
  std::weak_ptr<Sqlite> self_;

  TF_DISALLOW_COPY_AND_ASSIGN(Sqlite);
};

}  // namespace tensorflow

#endif  // TENSORFLOW_CORE_LIB_DB_SQLITE_H_

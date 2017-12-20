# Description:
#   The world's most popular SQL database in one .c file.

licenses(["unencumbered"])  # Public Domain

SQLITE_COPTS = [
    "-DHAVE_DECL_STRERROR_R=1",
    "-DHAVE_INTTYPES_H=1",
    "-DHAVE_STDINT_H=1",
    "-DHAVE_INTTYPES_H=1",
    "-D_FILE_OFFSET_BITS=64",
    "-D_REENTRANT=1",
] + select({
    "@org_tensorflow//tensorflow:windows": [],
    "@org_tensorflow//tensorflow:windows_msvc": [],
    "@org_tensorflow//tensorflow:darwin": [
        "-DHAVE_GMTIME_R=1",
        "-DHAVE_LOCALTIME_R=1",
        "-DHAVE_USLEEP=1",
    ],
    "//conditions:default": [
        "-DHAVE_FDATASYNC=1",
        "-DHAVE_GMTIME_R=1",
        "-DHAVE_LOCALTIME_R=1",
        "-DHAVE_POSIX_FALLOCATE=1",
        "-DHAVE_USLEEP=1",
    ],
})

# Production build of SQLite library that's baked into TensorFlow.
cc_library(
    name = "org_sqlite",
    srcs = ["sqlite3.c"],
    hdrs = [
        "sqlite3.h",
        "sqlite3ext.h",
    ],
    copts = SQLITE_COPTS + [
        # The defines below turn off a great deal of functionality. This
        # ends up causing the compiler to emit a whole bunch of warnings
        # about unused variables and functions. So we make those go away
        # with this flag, since we trust the SQLite authors.
        "-w",

        # This enables extremely conservative thread safety. We're
        # talking so conservative that even a BEGIN TRANSACTION query
        # will lock out any other thread that might have access to a
        # connection object, until a COMMIT or ROLLBACK happens.
        #
        # TODO(jart): See if we're better off setting this to 2.
        "-DSQLITE_THREADSAFE=1",

        # Enabling this feature means that when a DELETE happens, it
        # actually deletes the data, i.e. overwrites the row data with
        # zeros. This obviously carries a small performance tradeoff,
        # but it's totally worth it, since we're very serious about
        # being good stewards of user data.
        "-DSQLITE_SECURE_DELETE",

        # This allows us to say sqlite3_open("file:foo.sqlite?mode=ro").
        # This is nice, because it allows our APIs to be simpler, and we
        # might choose to create abstract interfaces that support
        # multiple databases in the future.
        "-DSQLITE_USE_URI",

        # According to the SQLite documentation, allowing LIKE queries
        # on BLOB values was a mistake they unintentionally made in the
        # past, that resulted in weird behaviors. So we're just going to
        # turn that weirdness off, since we're starting fresh.
        "-DSQLITE_LIKE_DOESNT_MATCH_BLOBS",

        # There's a chance that this feature could be useful to us in
        # the future, but until we understand more about what it does
        # and how it can benefit our use case, we'll exclude it.
        "-DSQLITE_OMIT_AUTHORIZATION",

        # Monotonic permanent IDs is an anti-pattern that doesn't adapt
        # to distributed systems. We have our own system in place for
        # generating random IDs. However, even with this functionality
        # disabled, our "INTEGER PRIMARY KEY" (AKA ROWID) fields still
        # get magically assigned numbers. The catch is that SQLite will
        # recycle those numbers. This is perfectly fine for us, because
        # our schema makes it very clear that rowids to be treated as
        # ephemeral, i.e. generally only valid within transactions,
        # unless a more specific definition is provided. This is because
        # rowids determines where row data is stored in SQLite's b-trees
        # and we want to preserve the flexibilty to write batch jobs
        # that shuffle things around in order to optimize locality.
        "-DSQLITE_OMIT_AUTOINCREMENT",

        # Since we have a write-heavy use case, we want to be able to do
        # clever things like preallocate rows with zeroblob to avoid
        # b-tree churn. In theory, auto vacuuming could break this plan.
        # Cursory testing has also been performed with this feature, and
        # we haven't been able to figure out the benefits so far.
        #
        # The consequence to omitting this functionality, is if we
        # connect to a DB where the user turned this feature on, it's
        # going to go into readonly mode. Since thes SQLite dataset ops
        # currently only do reads, this shouldn't be a problem.
        #
        # We are open to removing this define if someone files a bug
        # with the TensorBoard team that helps us explain why this
        # functionality will be helpful to both them and the community.
        "-DSQLITE_OMIT_AUTOVACUUM",

        # This is a production build, and production qureies should be
        # very well planned out to have all the explicitly defined
        # indexes they need in order to be tractable. So we're not yet
        # sure why it would be helpful for the SQLite to plan magic
        # temporary indexes to help us. However, this functionality is
        # still made available in @org_sqlite//:sqlite3 for when we need
        # to do heavy lifting maintenance. Another option is to simply
        # export the database to BigQuery.
        "-DSQLITE_OMIT_AUTOMATIC_INDEX",

        # The @org_sqlite//:sqlite3 command can be used to explain
        # queries. We might enable this in the future, possibly for a
        # testing build that's similar to the production build.
        "-DSQLITE_OMIT_EXPLAIN",

        # This SQLite build is statically linked into the monolithic
        # TensorFlow C++ library so all our extensions should be
        # statically linked as well. Please note that this doesn't
        # preclude the databases we create from being loaded into other
        # SQLite environments, e.g. Python, where arbitrary extension
        # loading is possible. It's also worth mentioning that it's easy
        # to add static extensions to TensorFlow's build. See sqlite.bzl
        "-DSQLITE_OMIT_LOAD_EXTENSION",

        # Google uses UTF8 for everything. In certain situations, we
        # even treat BLOB and TEXT as interchangeable. We also check to
        # make sure that the database a user gives us is in UTF-8 mode.
        # This is because choosing UTF-16 is permanent. Even if we
        # connect to a user's UTF-16 database, and specify the UTF-8
        # pragma, SQLite won't magically convert the strings we query to
        # UTF-8.
        #
        # We COULD in theory support UTF-16. But for the reasons above
        # it would require significant eng effort and subtle dangerous
        # mistakes could easily be made. If there's anyone out there who
        # strongly feels UTf-16 support has merits, please file an issue
        # with the TensorBoard team. We are very open to learning why
        # that might be the case.
        "-DSQLITE_OMIT_UTF16",

        # The SQLite documentation says this is some type of weird
        # legacy thing, so we're just going to turn it off.
        "-DSQLITE_OMIT_GET_TABLE",

        # This might be useful for folks writing GUIs that allow users
        # to type in their own SQL queries, and get some helpful
        # feedback on them. But we probably don't need this.
        "-DSQLITE_OMIT_COMPLETE",

        # This is another feature that would be useful for building some
        # type of SQL GUI, which we're currently not doing.
        "-DSQLITE_OMIT_PROGRESS_CALLBACK",

        # The guy who invented SQLite loves TCL. It might be cool thing
        # to use if it was something we understood. But until then, we
        # probably don't need it compiled into TensorFlow.
        "-DSQLITE_OMIT_TCL_VARIABLE",

        # # SQLite supports full-text search and we're totally going to
        # # want to enable this at some point in the future. But until
        # # that day arrives, we can make the binary 50-100kb smaller by
        # # not enabling it.
        # # https://sqlite.org/fts3.html
        # "-DSQLITE_ENABLE_FTS4",

        # Foreign keys with things like cascading deletes can be very
        # helpful for rigidly defined business databases, which makes
        # them much stricter, tidier, and easier to manage. That comes
        # with performance tradeoffs, particularly since we do secure
        # deletes. It also makes one more dependent on transactions.
        #
        # With TensorFlow, particularly with our write-heavy use case,
        # we'd like to it makes more sense to play a bit looser with the
        # data model, and then have tooling to collect orphan data.
        # Particularly considering with the way we do permenant IDs
        # alongside INSERT OR REPLACE.
        #
        # Foreign keys and cascades are recommended to be used only
        # sparingly. For example, it might be appropriate for sidekick
        # tables that have relationship on rowids rather than permanent
        # IDs; because in those cases, the orphoned data left behind can
        # be accidentally reused which leads to an incorrect state.
        #
        # "-DSQLITE_OMIT_FOREIGN_KEY",

        # This makes SELECT COUNT(*) queries run faster. Based on a
        # cursor examination of the SQLite implementation, that might
        # come at the cost of doing a bunch of extra writes up the btree
        # pages. There's some concern this might be might bad for our
        # write-heavy use case, but it might not be so bad since we're
        # pre-allocating empty zeroblob() inserts in transactional
        # batches, so it might not be sad. So we're going to err on the
        # side of trusting the SQLite authors for now.
        #
        # "-DSQLITE_OMIT_BTREECOUNT",

        # This is a necessary feature if we want to have multiple
        # threads in the same process, each with their own Sqlite
        # connection object, trying to read and write to the same DB
        # file. It will allow them all to access the same cross-thread
        # memory cache. But we currently don't appear to be doing this,
        # and it would require writing some mutex condition stuff ir our
        # Sqlite wrapper. See https://sqlite.org/unlock_notify.html
        #
        # "-DSQLITE_ENABLE_UNLOCK_NOTIFY",
    ],
    defines = [
        # This gets rid of the bloat of deprecated functionality. It
        # needs to be listed here instead of copts because it's actually
        # referenced in the sqlite3.h file.
        "SQLITE_OMIT_DEPRECATED",
    ],
    linkopts = select({
        "@org_tensorflow//tensorflow:windows_msvc": [],
        "//conditions:default": [
            "-ldl",
            "-lpthread",
        ],
    }),
    visibility = ["//visibility:public"],
)

# SQLite command line tool. This contains all the features we turn off
# in our production build, like the ability to EXPLAIN queries. This
# binary is also tuned to be more capable of performing computationally
# expensive operations.
#
#   bazel build @org_sqlite//:sqlite3
#   ./bazel-bin/external/org_sqlite/sqlite3 ~/.tensorboard.sqlite
#
cc_binary(
    name = "sqlite3",
    srcs = [
        "shell.c",
        "sqlite3.c",
        "sqlite3.h",
    ],
    copts = SQLITE_COPTS + [
        "-DSQLITE_THREADSAFE=2",
        "-DSQLITE_DEFAULT_WORKER_THREADS=4",
        "-DSQLITE_MAX_WORKER_THREADS=32",
        "-DSQLITE_DEFAULT_CACHE_SIZE=25000",  # ~100MB
        "-DSQLITE_ENABLE_EXPLAIN_COMMENTS",
        "-DSQLITE_SECURE_DELETE",
        "-DSQLITE_USE_URI",
        "-DSQLITE_OMIT_UTF16",
    ],
    linkopts = select({
        "@org_tensorflow//tensorflow:windows_msvc": [],
        "//conditions:default": [
            "-ldl",
            "-lpthread",
            "-lreadline",
            "-lncurses",
        ],
    }),
    visibility = ["//visibility:public"],
)

# Any Python library that imports sqlite3 needs to depend on this build
# rule; even though in the open source world, this doesn't really do
# anything. It's mostly an artifact of Google's internal sync process.
# However, in the future, we may choose to use this build rule to
# override the stock SQLite that comes included with Python.
py_library(
    name = "python",
    srcs_version = "PY2AND3",
    visibility = ["//visibility:public"],
)

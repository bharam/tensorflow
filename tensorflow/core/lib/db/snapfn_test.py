# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import sqlite3

from third_party.tensorflow.python.framework import test_util
from third_party.tensorflow.python.platform import resource_loader
from third_party.tensorflow.python.platform import test


class SqliteSnappyTest(test_util.TensorFlowTestCase):

  def setUp(self):
    super(SqliteSnappyTest, self).setUp()
    self.db = sqlite3.connect(':memory:')
    self.db.enable_load_extension(True)
    self.db.execute("select load_extension('%s')" %
                    resource_loader.get_path_to_datafile('libsnapfn.so'))
    self.db.enable_load_extension(False)

  def testActuallyWorks(self):
    self.assertNotEqual('hello',
                        self.db.execute('SELECT snap(\'hello\')')
                        .fetchone()[0])
    self.assertEqual(
        'hello',
        self.db.execute(
            'SELECT CAST(unsnap(snap(CAST(\'hello\' AS BLOB))) AS TEXT)')
        .fetchone()[0])
    self.assertEqual(
        'text',
        self.db.execute('SELECT typeof(unsnap(snap(\'h\')))').fetchone()[0])
    self.assertEqual(
        'blob',
        self.db.execute(
            'SELECT typeof(unsnap(snap(CAST(\'h\' AS BLOB))))').fetchone()[0])

  def testRoundTrip(self):
    self.assertEqual('hello',
                     self.db.execute('SELECT unsnap(snap(\'hello\'))')
                     .fetchone()[0])
    self.assertEqual(
        'hello',
        self.db.execute(
            'SELECT CAST(unsnap(snap(CAST(\'hello\' AS BLOB))) AS TEXT)')
        .fetchone()[0])
    self.assertEqual(
        'text',
        self.db.execute('SELECT typeof(unsnap(snap(\'h\')))').fetchone()[0])
    self.assertEqual(
        'blob',
        self.db.execute(
            'SELECT typeof(unsnap(snap(CAST(\'h\' AS BLOB))))').fetchone()[0])

  def testNull_passesThrough(self):
    self.assertIsNone(
        self.db.execute('SELECT unsnap(snap(NULL))').fetchone()[0])

  def testEmpty_passesThrough(self):
    self.assertEqual('',
                     self.db.execute('SELECT unsnap(snap(\'\'))').fetchone()[0])
    self.assertEqual(
        'text',
        self.db.execute('SELECT typeof(unsnap(snap(\'\')))').fetchone()[0])
    self.assertEqual(
        'blob',
        self.db.execute(
            'SELECT typeof(unsnap(snap(CAST(\'\' AS BLOB))))').fetchone()[0])

  def testBinaryCompatibility(self):
    self.assertEqual(
        'today is the end of the republic',
        self.db.execute(
            'select unsnap(X\'03207C746F6461792069732074686520656E64206F6'
            '6207468652072657075626C6963\')').fetchone()[0])

  def testInt_doesNothing(self):
    self.assertEqual(123, self.db.execute('SELECT snap(123)').fetchone()[0])
    self.assertEqual(123, self.db.execute('SELECT unsnap(123)').fetchone()[0])

  def testFloat_doesNothing(self):
    self.assertEqual(3.14, self.db.execute('SELECT snap(3.14)').fetchone()[0])
    self.assertEqual(3.14, self.db.execute('SELECT unsnap(3.14)').fetchone()[0])


if __name__ == '__main__':
  test.main()

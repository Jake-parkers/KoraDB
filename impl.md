# General Overview

The implementation of koradb was inspired heavily by [leveldb](https://github.com/google/leveldb) and [kingdb](https://github.com/goossaert/kingdb).

Each database is stored as a set of files consisting of a log file and multiple sstables.

# How it works

## Writing to the db

When a write come in, two things happen:

- the data (key and value) is written to an in memory tree structure called a memtable (specifically, this project uses std::map which uses a red-black tree under the hood). This data structure maintains the keys in a sorted order.
- the data (key and value) is also written to a log file in an append only manner. The reason for also writing to a log file is so that when in an event where the database crashes, the most recent writes in the memtable (which would be lost) can be restored from the log file.

Now, since this is a persistent key value database, we can't of course keep all of the data in the memtable. When the memtable gets to a specific size, the data in the memtable is written out to a file on disk (called an sstable) maintaining the sorted order of the data. Writing out to an sstable happens in a separate thread, thus, new writes to the db can continue to a new memtable instance. Every new sstable will be the most recent segment of the database.

## Reading from the db

When data is to be read from the db, there are 2 places to look
- the current memtable
- all of the sstables (database segments) that have been written out to disk

More recent writes that have not been written out to an sstable will reside in the memtable and so when a read request comes in, the result would be gotten from the memtable, otherwise, we have to search through all of the segment files starting from the most recent until the key is located or not (if it doesn'e exist).

## Deleting from the db

Deleting from the db doesn't immediately delete the key-value pair as that would be inefficient i.e having to delete from the memtable and possible other instance of that pair that may occur in any of the database segments.

What is done instead is this:
- a special record call a tombstone is added to the memtable and the logfile as well for that key-value pair. In the case of koradb, the key retains it's data but the value associated with that key is replaced with the tombstone record. Hence when a read request comes in after a delete request (and before compaction has occurred which would actually remove the data from the database segment files), since the most recent "write" to the db for the key to be read has the tombstone record as it's value, it will be reported as deleted.

This works works efficiently because it is either
1. The key-value pair that has been deleted resides in the current memtable with the tombstone record or
2. The key-value pair that has been deleted resides in one of the more recent segment file.
So when a read request comes in, the most recent record for that pair in the db (whether in the current memtable or in the segment files) would be read and recognized as deleted by the storage engine.

Every occurrence of the record will eventually be removed from the segment files the next time merging and compaction of segment files happen.

## Compaction

Compaction occurs in a background thread and it runs at specific intervals. The compaction picks any two files (sstables) that fall within the same range. Currently there are four (4) different size classes:
- 2MB <= size <= 5MB
- 5MB <= size <= 8MB
- 8MB <= size <= 12MB
- >= 12MB

Files that fall within the same range and compacted and merged together. If a key occurs in both files, the most recent write (which will reside in the most recent) sstable will be used. If the most recent write contains the tombstone record, the key-value pair would be deleted from that file and a search would begin to find all other occurences of that pair in other segment files and delete them accordingly. If otherwise no tombstone record is found, the the value of the pair in the most recent segment would be used.

## Misc

When the database is restarted, if there are any data left in the log file that have not been written out to an sstable, the data is in the log file is loaded into the current memtable so that it can eventually be written out to disk when the memtable gets to the set size limit. It should be noted however that the log file is not cleared until all data has been written out to an sstable.




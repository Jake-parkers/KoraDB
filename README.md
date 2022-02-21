# KoraDB (In Development)
Kora is a persistent key-value database that provides ordered mapping between key-value pairs. 

## Features

- Data stored is sorted by key

- Keys and values are stored as bytes

- The supported ops are Get(key), Set(key, value), Delete(key)

- The compaction strategy used is size-tiered compaction (compaction is done in the background periodically)

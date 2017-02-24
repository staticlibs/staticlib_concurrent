Staticlibs Concurrent library
=============================

This project is a part of [Staticlibs](http://staticlibs.net/).

This project contains the following concurrency-related templates:

 - `spsc_concurrent_queue` single producer single consumer non-blocking queue implementation based
on [ProducerConsumerQueue from facebook/folly](https://github.com/facebook/folly/blob/b75ef0a0af48766298ebcc946dd31fe0da5161e3/folly/ProducerConsumerQueue.h)
 - `mpmc_blocking_queue` optionally bounded growing FIFO blocking queue with support for blocking and 
non-blocking multiple consumers and always non-blocking multiple producers

This library is header-only and has no dependencies.

Link to the [API documentation](http://staticlibs.github.io/staticlib_containers/docs/html/namespacestaticlib_1_1containers.html).

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2017-02-24**

 * initial public version

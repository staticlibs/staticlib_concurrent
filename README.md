Staticlibs Concurrent library
=============================

This project is a part of [Staticlibs](http://staticlibs.net/).

This project contains the following concurrency-related templates:

 - single producer single consumer non-blocking (wait-free) FIFO queues based
on [ProducerConsumerQueue from facebook/folly](https://github.com/facebook/folly/blob/b75ef0a0af48766298ebcc946dd31fe0da5161e3/folly/ProducerConsumerQueue.h):
  * `spsc_concurrent_queue` wait-free queue with fixed-size heap storage
  * `spsc_waiting_queue` the same as previous one with optional blocking `take` operation
  * `spsc_inobject_concurrent_queue` wait-free queue with fixed-size in-object 
(on-stack for stack allocated queue) storage
  * `spsc_inobject_waiting_queue` the same as previous one with optional blocking `take` operation
 - `mpmc_blocking_queue` optionally bounded growing FIFO blocking queue with support for blocking and 
non-blocking multiple consumers and always non-blocking multiple producers
 - `condition_latch` spurious-wakeup-free lock that uses arbitrary "condition" functor to check locked/unlocked state
 - `countdown_latch` synchronization aid that allows one or more threads to wait until a set
of operations being performed in other threads completes
 - `growing_buffer` non-shrinkable `char` heap buffer with non-destructive `move` (the same as `copy`) logic,
grows if needed on `move-in` operation

All queues and locks are non-copyable, non-movable and inherit from [enable_shared_from_this](http://en.cppreference.com/w/cpp/memory/enable_shared_from_this).
to be used inside `std::shared_ptr`.

This library is header-only and has no dependencies.

Link to the [API documentation](http://staticlibs.github.io/staticlib_concurrent/docs/html/namespacestaticlib_1_1concurrent.html).

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2017-02-24**

 * initial public version

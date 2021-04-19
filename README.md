# radixsort
Yet another radix sort.

This one was developed working on some BI/BigData project for quite special internals. So, I generalized code somewhat and put it here for everyone interested.
The code IS fast. However, keep in mind, that you want to use FAST containers with this sort, and std::vector isn't all that good. We used our own containers implemented over anonymous mmap - mostly because we were sorting millions to billions of items at once.
There is multithreaded implementation as well, of course (in fact, THREE different multithreaded implementations), but they're not very useful in production as they tend to totally strangle servers, givinig just about x4 sorting speed, if memory serves well.

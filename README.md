# timeq

A time based queue, where the length of the queue is a duration,
divided into buckets based on a given time interval. As time
progresses, buckets in the past are cleared, and the main queue
is updated so that the front only returns a valid object that
has not expired. To improve performance, buckets are only cleared
on push or pop operations. Thus, buckets in the past can be
cleared in bulk based on how many we should have advanced since
the last time we updated.
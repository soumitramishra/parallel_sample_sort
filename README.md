# parellel_sample_sort
Implementation of parellel sample sort algorithm 
<p>
Sample sort is a parallel version of quicksort. </br>

It works as follows:</br>

<b>Input:</b></br>
</br>
<ul>
<li> An array of N items
<li> An integer P for number of processes to use.
</ul>
</b>Result:</b></br>
</br>
<ul><li>The input array has been sorted.</ul></br>
</p>

<p>
<b>Steps:</b>
<ol>
<li> Sample Randomly select 3*(P-1) items from the array.
<li> Sort those items.
<li> Take the median of each group of three in the sorted array, producing an array (samples) of (P-1) items.
<li> Add 0 at the start and +inf at the end (or the min and max values of the type being sorted) of the samples array so it has (P+1) items numbered (0 to P).
</ol>
Partition
<ol>
<li> Spawn P processes, numbered p in (0 to P-1).
<li> Each process builds a local array of items to be sorted by scanning the full input and taking items between samples[p] and samples[p+1].
<li> Write the number of items (n) taken to a shared array sizes at slot p.
<li> Sort locally.
Each process uses quicksort to sort the local array.
<li> Copy local arrays to input.
</ol>

Each process calculates where to put its result array as follows:
<ul>
<li> start = sum(sizes[0 to p - 1]) # that’s sum(zero items) = 0 for p = 0
<li> end = sum(sizes[0 to p]) # that’s sum(1 item) for p = 0. Warning: Data race if you don’t synchronize here.
<li>Each process copies its sorted array to input[start..end]
</ul>
Cleanup

<ul><li>Terminate the P subprocesses. Array has been sorted “in place”.</ul>
</p>

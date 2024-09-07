# sem_vs_condsafe

**Authors:**
Esteban Solís Alfaro C37670
Josué Torres Sibaja C37853

## When using 32.500 threads

| Condsafe | Sempahore | Pure Busy Wait | Busy Wait with Delay | Pseudo-random Busy Wait |
|:------------: |:--------------:| :-------------:| :-------------:| :-------------:|
| 1,6834secs | 1,6196secs | 4secs (19000 threads)| 2,6784secs  | 2,8563secs |
| 1,7135secs | 1,6635secs | ... | 2,7835  | 2,9010secs |
| 1,7327secs | 1,5993secs | ... | 2,7145  | 2,8634secs |

### Notes

* Using max threads with the Condsafe takes 1,7 seconds to get to 32500. (Beacuse of allocs, the use of more than 32.500 threads breaks the program)
* Using Semaphore with max threads takes 1,5 seconds to get to 32500.
* Using the Pure busy_wait takes 4secs to get to thread number 19000 of 32500  (Computer was running super slow when stopped)
* Using the Busy_wait with delay (50 microseconds) takes 2,7secs to get to 32500.
* Using the Pseudo random busy_wait takes 2,8secs to get to 32500

### Conclusion

If we had to choose one for a program, we would pick Semaphore, because it's faster than the other options and is very versatile.

The reason why the busy wait takes longer than Semaphore and Conditionally safe is because these last two don't waste CPU resources like busy wait does.  

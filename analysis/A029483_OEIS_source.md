A029483

Numbers k that divide the (left) concatenation of all numbers <= k written in base 14 (most significant digit on left).

1, 13, 143, 169, 221, 403, 587, 11219, 178357, 222157, 85762339, 1086336563, 8005332049, 15081597011

([list](https://oeis.org/A029483/list); [graph](https://oeis.org/A029483/graph); [refs](https://oeis.org/search?q=A029483+-id:A029483); [listen](https://oeis.org/A029483/listen); [history](https://oeis.org/history?seq=A029483); [text](https://oeis.org/search?q=id:A029483&fmt=text); [internal format](https://oeis.org/A029483/internal))

OFFSET

1,2

COMMENTS

No other terms below 3\*10^10.

LINKS

[Table of n, a(n) for n=1..14.](https://oeis.org/A029483/b029483.txt)

[Index entries for related sequences](https://oeis.org/index/N#concat)

MATHEMATICA

b = 14; c = {}; Select\[Range\[10^4\], Divisible\[FromDigits\[c = Join\[IntegerDigits\[#, b\], c\], b\], #\] &\] (\* [Robert Price](https://oeis.org/wiki/User:Robert_Price), Mar 12 2020 \*)

CROSSREFS

Cf. [A029447](https://oeis.org/A029447 "Numbers k that divide the (right) concatenation of all numbers <= k written in base 2 (most significant digit on left).")\-[A029470](https://oeis.org/A029470 "Numbers k that divide the (right) concatenation of all numbers <= k written in base 25 (most significant digit on left)."), [A029471](https://oeis.org/A029471 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 2 (most significant digit on left).")\-[A029494](https://oeis.org/A029494 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 25 (most significant digit on left)."), [A029495](https://oeis.org/A029495 "Numbers k such that k divides the (right) concatenation of all numbers <= k written in base 2 (most significant digit on right).")\-[A029518](https://oeis.org/A029518 "Numbers k such that k divides the (right) concatenation of all numbers <= k written in base 25 (most significant digit on ri..."), [A029519](https://oeis.org/A029519 "Numbers k such that k divides the (left) concatenation of all numbers <= k written in base 2 (most significant digit on righ...")\-[A029542](https://oeis.org/A029542 "Numbers k such that k divides the (left) concatenation of all numbers <= k written in base 25 (most significant digit on rig..."), [A061931](https://oeis.org/A061931 "Numbers n such that n divides the (right) concatenation of all numbers <= n written in base 2 (most significant digit on right).")\-[A061954](https://oeis.org/A061954 "Numbers n such that n divides the (right) concatenation of all numbers <= n written in base 25 (most significant digit on ri..."), [A061955](https://oeis.org/A061955 "Numbers n such that n divides the (left) concatenation of all numbers <= n written in base 2 (most significant digit on right).")\-[A061978](https://oeis.org/A061978 "Numbers n such that n divides the (left) concatenation of all numbers <= n written in base 25 (most significant digit on right).").

Sequence in context: [A221103](https://oeis.org/A221103 "Number of nX2 arrays of occupancy after each element moves to some king-move neighbor, without 2-loops.") [A239250](https://oeis.org/A239250 "Number of nX2 0..4 arrays with no element equal to one plus the sum of elements to its left or one plus the sum of elements ...") [A386612](https://oeis.org/A386612 "a(n) = Sum_{k=0..n-1} binomial(4*k+1,k) * binomial(4*n-4*k,n-k-1).") \* [A266806](https://oeis.org/A266806 "Coefficient of x^2 in the minimal polynomial of the continued fraction [1^n,sqrt(6),1,1,...], where 1^n means n ones. S.") [A015672](https://oeis.org/A015672 "Expansion of e.g.f. theta_3^(13/2).") [A234601](https://oeis.org/A234601 "Total counts of distinct (undirected) cycles for all simple graphs of order n.")

Adjacent sequences: [A029480](https://oeis.org/A029480 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 11 (most significant digit on left).") [A029481](https://oeis.org/A029481 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 12 (most significant digit on left).") [A029482](https://oeis.org/A029482 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 13 (most significant digit on left).") \* [A029484](https://oeis.org/A029484 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 15 (most significant digit on left).") [A029485](https://oeis.org/A029485 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 16 (most significant digit on left).") [A029486](https://oeis.org/A029486 "Numbers k that divide the (left) concatenation of all numbers <= k written in base 17 (most significant digit on left).")

KEYWORD

nonn,base,more

AUTHOR

[Olivier Gérard](https://oeis.org/wiki/User:Olivier_Gérard)

EXTENSIONS

More terms from Larry Reeves (larryr(AT)acm.org), Jul 09 2001

Edited and updated by Larry Reeves (larryr(AT)acm.org), Apr 12 2002

a(11) from [Max Alekseyev](https://oeis.org/wiki/User:Max_Alekseyev), May 15 2011

a(12)-a(14) from [Jason Yuen](https://oeis.org/wiki/User:Jason_Yuen), Jun 04 2024

STATUS

approved

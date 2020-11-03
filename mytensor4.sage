from scipy.sparse import csr_matrix
load('koszul.sage')
load('sms.py')

#m3 br 17 ex
a = [0]*15
a[0] = { (2,8): 1}
a[1] = { (2,6): 1,(5,7): 1,(8,8): 1}
a[2] = { (1,6): 1,(4,7): 1,(7,8): 1}
a[3] = { (0,6): 1,(3,7): 1,(6,8): 1}
a[4] = { (7,7): 1}
a[5] = { (2,7): 1}
a[6] = { (1,7): 1}
#a[7] = { (2,3): 1,(5,4): 1,(8,5): 1}
a[7] = { (1,3): 1,(4,4): 1,(7,5): 1}
a[8] = { (0,3): 1,(3,4): 1,(6,5): 1}
a[9] = { (5,2): 1}
a[10] = { (4,2): 1}
a[11] = {  (2,2): 1}
a[12] = {  (1,2): 1}
#a[14] = {  (2,0): 1,(5,1): 1,(8,2): 1}
a[13] = { (1,0): 1,(4,1): 1,(7,2): 1}
a[14] = {  (0,0): 1,(3,1): 1,(6,2): 1}



# Constructs tensor as list of 18 matrices (via list comprehension),
# syntax for matrix command is matrix(dimB, dimC, Dictionary_of_values),
# matrices here will be over Z, since each entry is integer, if you wish
# to make over Q, then syntax will be matrix(QQ, dimB, dimC, Dictionary_of_values)
T = [ matrix(9,9,a[i]).transpose() for i in range(15)]



for p in range(0,7):
	print('p is', p)
	M = TAp(genericproject(T,2*p+1),p)
	r = linbox_rank(csr_matrix(M))
	print('rk is',r)
	a = 2*p+1
	b = binomial(a-1,p)
	d = ceil(r/b)
	print('koszul bound is',d)

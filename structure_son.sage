#load('borderapolarity3.sage')
load('sms.py')
load('koszul.sage')
from scipy.sparse import csr_matrix

def e(i,j,n):
    return matrix(QQ,n,n,{(i,j):1})

def son_basis(k,n):
    assert k < floor(n*(n-1)/2)
    l = floor(n/2)
    if k < l**2: #A block
        i,j = k//l, k%l
        return e(i,j,n) - e(l+j,l+i,n)
    elif k < l**2+(l*(l-1)/2): #B
        k -= l**2
        i = l - 2 - floor(sqrt(-8*k + 4*l*(l-1)-7)/2.0 - 0.5)
        j = k + i + 1 - l*(l-1)/2 + (l-i)*((l-i)-1)/2 #triangular idx
        return e(i,j+l,n)-e(j,i+l,n)
    elif k < l**2 + l*(l-1): #C
        k -= l**2 + floor(l*(l-1)/2)
        i = l - 2 - floor(sqrt(-8*k + 4*l*(l-1)-7)/2.0 - 0.5)
        j = k + i + 1 - l*(l-1)/2 + (l-i)*((l-i)-1)/2 
        return e(i+l,j,n)-e(j+l,i,n)
    else:
        k -= l**2 +l*(l-1)
        j = (k+l)%(n-1)
        return e(n-1,k,n)-e(j,n-1,n)

def Tson(n):
    d = floor(n*(n-1)/2)
    T = [{} for i in range(d)]
    B = matrix(QQ,[ son_basis(i,n).list() for i in range(d)])
    for i in range(d):
        a = son_basis(i,n)
        for j in range(d):
            b = son_basis(j,n)
            c = a*b - b*a
            c = -c.T
            c = B.solve_left(vector(QQ,c.list()))
            for k in c.nonzero_positions():
                T[i][(j,k)] = c[k]
    T = [matrix(QQ,d,d,m) for m in T]
	#reps = []
	#C = matrix(QQ,n,n)
    return T


def koszul_summary(T,pmin,pmax):
    print('#######################')
    for p in range(pmin,pmax+1):
        print('p is '+str(p))
        a = 2*p+1
        M = TAp(genericproject(T,a),p)
	print('Dimensions are '+str(M.dimensions()))
        r = linbox_rank(csr_matrix(M))
        print('rk is '+str(r))
        b = binomial(a-1,p)
        d = ceil(r/b)
        print('koszul bound is '+str(d))
    return


## so(4)
T = Tson(4)
print('so(4)')
koszul_summary(T,0,2)
#basicsummarize(T,2,3)

## so(5)
T = Tson(5)
#koszul_summary(T,0,4)
#basicsummarize(T,4,5)

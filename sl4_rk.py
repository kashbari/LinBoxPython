import numpy as np
#import sympy as sp
import scipy as scipy
from scipy import sparse
import math
from itertools import *
#from sympy import Matrix
#from scipy.sparse.linalg import inv



def nCr(n,r):
    f = math.factorial
    return f(n) / f(r) / f(n-r)
    
def a(x,y):
    a = np.zeros( (int(nCr(x,y+1)), int(nCr(x,y)) * x) )
    pdict = {s:i for i,s in enumerate(combinations(range(x),y))}
    pp1dict = {s:i for i,s in enumerate(combinations(range(x),y+1))}

    for S in combinations(range(x),y):
        for j in range(x):
            if j in S: continue
            Sp = tuple(sorted(S + (j,)))
            ix = pp1dict[Sp]
            jx = pdict[S] * x + j
            sgn = (-1) ** Sp.index(j)
            a[ix,jx] = sgn
    return a    
    
    
''' COMPUTE STRUCTURE TENSOR OF SL4 LIE ALGEBRA '''
n = int(input("n = "))
m=(n**2)-1

def ind(x):
    a = int(np.floor(x/n))
    b= x % n
    c = [a,b]
    return c

def iind(x):
    if x == 0:
        c = -1
    else:
        c = n*x[0] +x[1]
    return c

''' Computes Lie bracket E = [m,n] a = [i,j] '''
def LB(E,a):
    c1 = 0
    c2 = 0
    if a[0] ==E[1]:
        c1 = [E[0],a[1]]
        if a[1] == E[0]:
            c2 = [a[0],E[1]]
        else:
            c2 = 0
    else:
        c1 = 0
        if a[1] == E[0]:
            c2 = [a[0],E[1]]
        else:
            c2 = 0
    c = [c1,c2]
    return c

def LB1(E,a):
    [c1,c2] = LB(E,a)
    if c1 == 0 and c2 == 0:
        d = print(0)    
    elif c1 == 0 and c2 != 0:
        d = print("-",c2, sep="")
    elif c2 == 0 and c1 != 0:
        d = print(c1)
    else:
        d = print(c1,c2,sep="-")
    return d


def LBD(E,a):
    if a[0] == a[1]:
        c = LB(E,a)
        a1 = [a[0]+1,a[1]+1]
        d = LB(E,a1)
        c = c+ [d[1],d[0]]
    else:
        c=LB(E,a)
    return c



''' Computes Lie bracket E = [m,n] a = [a1,a2,...] = [i,j,k,l,...], returns + - + - '''
def replace(q,v,k):
    q1 = q
    q1[k] = v[0]
    q1[k+1] = v[1]
    return q1

def LBT(E,a):
    b = []
    l = len(a)
    l2 = int(l/2)
    for k in range(0,l2):
        a1 = [a[2*k],a[2*k+1]]
        L = LBD(E,a1)
        for i in range(0,len(L)):
            if L[i] != 0:
                q = replace(a,L[i],2*k)
                b = b + [q]
            else:
                b = b + [0]
    return b

def TL(x,y):
    x1 = ind(x)
    y1 = ind(y)
    z1 = LBT(x1,y1)
    z = [0]*len(z1)
    for i in range(len(z1)):
        z[i] = iind(z1[i])
    return z

''' Actually construct structure tensor '''
T = np.zeros((m,m,m))
for i in range(m):
    for j in range(m):
        z = TL(i,j)
        for k in range(len(z)):
            if z[k] == -1:
                T = T
            elif (z[k] >= m):
                T = T
            else:
                T[i,j,z[k]] =  T[i,j,z[k]] + (-1)**k
                


# COMPUTE THE RANK OF THE KOSZUL FLATTENING
p = int(input("p = "))


d = int(nCr(m,p))
# S: B* \to A \otimes C
Tf = T.reshape(m,m*m)
Tf = sparse.csr_matrix(Tf)
print(Tf.shape)
#print(np.linalg.matrix_rank(Tf))

# Id_Skew \otimes S: L^p A \otimes B^* \to L^p A \otimes A \otimes C
K = sparse.kron(np.eye(d), Tf)
print(K.shape)

#Projection L^p \otimes A to L^{p+1}A    
aa = a(int(m),int(p))
aa = sparse.csr_matrix(aa)
#Kronecker product with C
P = sparse.kron(aa.T,np.eye(m))
P = sparse.csr_matrix(P)
print(P.shape)
TAp = K @ P
print(TAp.shape)

from itertools import combinations
from bisect import *
from operator import *

# Tensor representation of A x B x C is a list of b x c matrices of length a

def symbolic_tensor(a):
    R=PolynomialRing(QQ,['X' + str(i) for i in [1..a]])
    # R=FreeAlgebra(QQ,'X',a)
    return [ matrix([[R.gens()[i]]]) for i in range(a) ]

def rank1_tensor(av,bv,cv):
    a,b,c = len(av), len(bv), len(cv)
    X = matrix(b,1,bv) * matrix(1,c,cv)
    return [ X * ai for ai in av ]

def generic_rank1_tensor(a,b,c):
    X = matrix.random(QQ,b,1) * matrix.random(QQ,1,c)
    return [ X * QQ.random_element() for i in range(a) ]

def symbolic_rank1_tensor(a,b,c):
    R = PolynomialRing(QQ,["a" + str(i) for i in [1..a] ] +\
            ["b" + str(i) for i in [1..b]] +\
            ["c" + str(i) for i in [1..c]])
    av = R.gens()[:a]
    bv = R.gens()[a:a+b]
    cv = R.gens()[a+b:]
    return rank1(av,bv,cv)

def TAp(T,p,sparse=True):
    a=len(T)
    b,c = T[0].dimensions()
    R = T[0].base_ring()
    That = {}
    for ii in range(a):
        for S in combinations(range(ii) + range(ii+1,a),p):
            ix = bisect_left(S,ii)
            sg = (-1)^ix
            P = S[:ix] + (ii,) + S[ix:]
            br = subsetix(P)
            bc = subsetix(S)
            for (i,j) in T[ii].nonzero_positions():
                That[(br*c + j,bc*b + i)] = sg * T[ii][i,j]
    return matrix(R,binomial(a,p+1)*c, binomial(a,p)*b,That,sparse=sparse)

# if dense output is desired, above should be better in most cases
def TApdense(T,p):
    a=len(T)
    b,c = T[0].dimensions()
    R = T[0].base_ring()
    That=matrix(R,binomial(a,p+1)*c, binomial(a,p)*b)
    for ii in range(a):
        for S in combinations(range(ii) + range(ii+1,a),p):
            ix = bisect_left(S,ii)
            sg = (-1)^ix
            P = S[:ix] + (ii,) + S[ix:]
            br = subsetix(P)
            bc = subsetix(S)
            # print ii,S,P,br,bc,'+' if sg == 1 else '-'
            for (i,j) in T[ii].nonzero_positions():
                That[br*c + j,bc*b + i] = sg * T[ii][i,j]
                # That[br*c + j,bc*b + i] += sg * T[ii][i,j]
    return That

def genericproject(T,newa,sparse='same',exact=True,denseintermediate=True):
    if exact:
        a = random_matrix(ZZ,newa,len(T),x=-10^8,y=10^8)
        # a = random_matrix(ZZ,newa,len(T),x=-10^16,y=10^16)
    else:
        a = random_matrix(RDF,newa,len(T))
    sparsestart = T[0].is_sparse()
    if denseintermediate: 
        T = [m.dense_matrix() for m in T]
    Tp = [ reduce(add,[ X * a[i,j] for j,X in enumerate(T) ]) for i in range(newa) ]
    if sparse == True or sparse == 'same' and sparsestart:
        Tp = [m.sparse_matrix() for m in Tp] 
    else:
        Tp = [m.dense_matrix() for m in Tp]
    return Tp

def symbolicproject(T,newa):
    R = PolynomialRing(T[0].base_ring(),'tt',len(T)*newa)
    # thought this might be useful, but sage cannot reliably find ranks of matrices
    # over such a polynomial ring
    return [ reduce(add,[ X * R.gen(j*newa+i) for j,X in enumerate(T)]) for i in range(newa) ]

# returns (rank(TAp), new restricted dimension of a)
def TAprank(T,p,exact=None,denseintermediate=False):
    exact = exact or T[0].base_ring().is_exact()
    a = len(T)
    if 2*p+1 >= a: # no projection needed
        M=TAp(T,p)
    else: # project to generic 2p+1 dimensional subspace
        a = 2*p+1
        M=TAp(genericproject(T,2*p+1,exact=exact),p)
    if exact:
        return M.rank(),a
    else:
        return inexactrank(M),a

def summary1(T,p,exact=None):
    r,a = TAprank(T,p,exact)
    d=binomial(a-1,p)
    return [p,r,d,ceil(r/d)]

def summary(T,pmin=0,pmax=4,**kwargs):
    for p in [pmin..pmax]:
        for t in range(kwargs.get('tries',1)):
            try:
                cur = summary1(T,p,kwargs.get('exact',None)) 
                yield cur
                if cur[1]/Integer(cur[2]) >= kwargs.get('bound',Infinity):
                    return
            except KeyboardInterrupt as e:
                raise e
            except:
                pass

def summarize(T,pmin=0,pmax=4,**kwargs):
    f=kwargs.get('f',sys.stdout)
    best=0
    for t in summary(T,pmin,pmax,**kwargs):
        best = max(best,t[1]/t[2])
        if 'prefix' in kwargs:
            t = kwargs['prefix'] + t
        f.write(kwargs.get('delim',',').join(map(str,t))+'\n')
        f.flush()
    return best

def basicsummarize(T,pmin,pmax):
    a=len(T)
    for p in [pmin..pmax]:
        M=TAp(T,p)
        r=M.rank()
        bound=r/binomial(a-1,p)
        print "p =",p,"TAp dimensions",M.dimensions(),"rank(TAp) =",r,
        print "border rank(T) >=",ceil(bound)

def subsetix(S):
    res=0
    for i in range(len(S)):
        res += binomial(S[i], i+1)
    return res

# vim: ft=python

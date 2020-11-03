import scipy.sparse

def tosms(m):
	I,J,V = scipy.sparse.find(m)
	out = '%d %d M\n' % m.shape
	out += ''.join(['%d %d %d\n' %(i+1,j+1,v) for i,j,v in zip(I,J,V)]) + '0 0 0\n'
	return out

# m is scipy any sparse format
def linbox_rank(m):
	import subprocess
	proc = subprocess.Popen(['/home/kashbari/git_repo/LinBoxPython/rank'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,shell=True)
	proc.stdin.write(tosms(m).encode())
	return int(proc.communicate()[0])




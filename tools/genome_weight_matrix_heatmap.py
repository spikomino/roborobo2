import pylab
from optparse import OptionParser
from optparse import OptionGroup

from genome2graph import *

# Process arguments
def read_options(defaults):
    parser = OptionParser()
    
    parser.add_option("-o", "--outfile", 
                      dest="outfile",
                      type="string",
                      default=defaults['outfile']   ,  
                      help="the filename for the dotfile [default "
                      +str(defaults['outfile'])+"]")

    parser.add_option("-p", "--path", 
                      dest="path",
                      type="string",
                      default=defaults['path']   ,  
                      help="the path to genome files [default "
                      +str(defaults['path'])+"]")

    return parser.parse_args()

# If run directly (toplevel)

if __name__ == '__main__':

    # Read options & provide with defaults
    defaults_opts={}
    defaults_opts['outfile']   = None 
    defaults_opts['path']      = 'test'
    (options, args) = read_options(defaults_opts)

    # read the directory (list the genome files)
    fl = []
    for f in os.listdir(options.path):
        if f.endswith('.gen'):
            fl.append(f)
    fl.sort()
    
    M=[]
    for f in fl :
        M.append(process_weight_matrix(options.path+'/'+f))
        
    # compute the pairewise distance 
    size = len(M)

    Z =  [[0 for x in xrange(size)] for x in xrange(size)]
    for x in xrange(size):
        for y in xrange(x,size):
            Z[x][y] = weight_matrix_dist(M[x], M[y])
   

    

    fig = pylab.figure() 
    ax = pylab.axes([0.025,0.025,0.95,0.95])
    pylab.imshow(Z,interpolation='nearest', cmap='spectral', origin='lower')
    pylab.colorbar(shrink=.92)
    
  
    # savefig('../figures/imshow_ex.png', dpi=48)

    def onclick(event):
        if  event.xdata > 0 or event.xdata < len(fl) or\
                event.ydata > 0 or event.ydata < len(fl) :
            g1 = int(event.xdata)
            g2 = int(event.ydata)
            print fl[g1], fl[g2]
        
    cid = fig.canvas.mpl_connect('button_press_event', onclick)

    pylab.show()
    

   

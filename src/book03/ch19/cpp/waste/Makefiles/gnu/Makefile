OBJS=asyncdns.o blowfish.o config.o connection.o d_chat.o filedb.o license.o listen.o m_chat.o m_file.o m_keydist.o m_lcaps.o m_ping.o m_search.o m_upload.o mqueue.o mqueuelist.o netkern.o sha.o sharedmain.o shbuf.o srchwnd.o srvmain.o util.o xfers.o xferwnd.o rsa/md5.o rsa/nn.o rsa/prime.o rsa/r_keygen.o rsa/r_random.o rsa/rsa.o

DEBUGFLAG = -g0 -s
OPTIMIZEFLAGS = -O2
DEFINES = -D_DEFINE_SRV -DNDEBUG
CXXFLAGS = $(OPTIMIZEFLAGS) $(DEBUGFLAG) $(DEFINES) -pipe -I. -Irsa -march=pentiumpro

default: wastesrv

wastesrv: $(OBJS)
	$(CXX) $(DEBUGFLAG) -pthread -o wastesrv $(OBJS)

clean:
	-rm -r *.o
	-rm wastesrv

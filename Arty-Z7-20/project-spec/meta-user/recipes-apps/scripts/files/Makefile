APP = scripts

# Add any other object files to this list below
APP_OBJS = 

all: install

build:
	cp -f -d libvrc.so $(STAGEDIR)/usr/lib/libvrc.so
	ln -f -s $(STAGEDIR)/usr/lib/libvrc.so.0 $(STAGEDIR)/usr/lib/libvrc.so
	cp -f -d libvrc.h $(STAGEDIR)/usr/include/libvrc.h

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

clean:

install: 
	$(TARGETINST) -pf libvrc.so /usr/lib/libvrc.so
	$(TARGETINST) -pf libvrc.h /usr/include/libvrc.h
	

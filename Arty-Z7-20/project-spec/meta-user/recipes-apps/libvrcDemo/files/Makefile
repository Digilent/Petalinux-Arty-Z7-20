APP = libvrcDemo

# Add any other object files to this list below
APP_OBJS = drmdemo.c libvrc.h libvrc.c

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

clean:
	rm -f $(APP) *.0 *~

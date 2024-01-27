TARGET = adc_udp_test_demo
CC = arm-rockchip830-linux-uclibcgnueabihf-gcc
CFLAGS = -Iinclude
DEPS = $(wildcard include/*.h)
SRCDIR = sources
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/*.o $(TARGET)

CC := arm-none-linux-gnueabi-gcc

TARGET := csi_test

.PHONY: all clean


all: $(TARGET)

csi_test:app_test_ok.c
	$(CC) $(CFLAGS) -static  $^  -o  $@

clean:
	rm -rf $(TARGET)

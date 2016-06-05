KOBJS  = kstart.o kuserblob.o kernel.o libc.o
UOBJS  = ustart.o umain.o libc.o

CFLAGS = -fno-builtin

all: kernel.code.hex kernel.data.hex kernel.user.hex

kernel.elf: user.code.bin user.data.bin $(KOBJS)
	sisa-ld -T kernel.ld $(KOBJS) -o $@
	@rm -f user.code.bin user.data.bin

user.elf: $(UOBJS)
	sisa-ld -T user.ld $^ -o $@

.c.o:
	sisa-gcc $(CFLAGS) -c $< -o $@

.s.o:
	sisa-as $< -o $@

%.hex: %.bin
	@hexdump -ve '1/2 "%.4x\n"' $< > $@

%.code.bin: %.elf
	sisa-objcopy -O binary -j .text $< $@

%.data.bin: %.elf
	sisa-objcopy -O binary -j .data $< $@

%.user.bin: %.elf
	sisa-objcopy -O binary --set-section-flags .user=alloc,load -j .user $< $@

kdisasm: kernel.elf
	@sisa-objdump -d $<

udisasm: user.elf
	@sisa-objdump -d $<

clean:
	@rm -f $(KOBJS) $(UOBJS) kernel.elf user.elf \
	kernel.code.hex kernel.data.hex kernel.user.hex \
	kernel.code.bin kernel.data.bin kernel.user.bin \
	user.code.bin user.data.bin \

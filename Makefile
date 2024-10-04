SDCC ?= sdcc
OUTDIR=build
APP=everled
ICNAME=PFS154
TARGET=$(OUTDIR)/$(APP)_$(ICNAME).ihx

all: $(TARGET)

$(TARGET): $(APP).c
	mkdir -p $(OUTDIR)
	$(SDCC) -D$(ICNAME) -mpdk14 -o $@ $<

clean:
	rm -rf $(OUTDIR)

flash: $(TARGET)
	easypdkprog --icname=$(ICNAME) write $<

run:
	easypdkprog --runvdd=3.0 start

.PHONY: all clean flash run

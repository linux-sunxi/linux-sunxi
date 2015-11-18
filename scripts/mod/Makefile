hostprogs-y	:= modpost mk_elfconfig
always		:= $(hostprogs-y) empty.o

ifdef KBUILD_SCRIPTROOT
MKELFCONFIG = $(KBUILD_SCRIPTROOT)/scripts/mod/mk_elfconfig
else
MKELFCONFIG = $(obj)/mk_elfconfig
endif

modpost-objs	:= modpost.o file2alias.o sumversion.o

# dependencies on generated files need to be listed explicitly

$(obj)/modpost.o $(obj)/file2alias.o $(obj)/sumversion.o: $(obj)/elfconfig.h

quiet_cmd_elfconfig = MKELF   $@
      cmd_elfconfig = $(MKELFCONFIG) < $< > $@

$(obj)/elfconfig.h: $(obj)/empty.o $(MKELFCONFIG) FORCE
	$(call if_changed,elfconfig)

targets += elfconfig.h

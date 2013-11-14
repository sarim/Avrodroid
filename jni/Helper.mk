define local-intermediates-dir
jni/external/v8/src
endef

define copy-file-to-target
@mkdir -p $(dir $@)
cp -fp $< $@
endef

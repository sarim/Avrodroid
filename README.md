Avrodroid
==
AvroKeyboard for android.

![Avro Android Screenshot](http://cl.ly/SQN6/screenshot-avrodroid.png)

Installation
==
This project is in very very early stage of development. Can do nothing at all. You cant even write a word with it now. So proceed with your own risk and dont ask for support.

###Place this first


commit `fc98e10624aa0fc5da76b5818d2cbcffe883e939` 
of `https://android.googlesource.com/platform/external/stlport`
in -> `jni/external/stlport/`

if compile gives error, revert to `e5f45d3719275b0fd752b94c3de2e6ca8bb097bb`

--

commit `c532760504e991c735370db73387337df5354de0` 
of `https://android.googlesource.com/platform/external/v8`
in -> `jni/external/v8/`


###Patch

	cd jni/external/v8/
	patch -p1 < ../../../gittu.patch


###Generating Snapshot [Optional]
To save time i've put the generated `snapshot.cc` in `/jni`, it is copied inside v8 source when building. If you are building v8 from another commit, maybe this snapshot wont be compatible. Run the `mksnapshot.sh` to regenerate the snapshot.cc. It is generated using `scons` So dont forget to run `brew install scons`. 


###Compile



Optional:
I have these in my bash, but how to setup ndk is up to you.

	export ANDROID_NDK_ROOT=/Users/sarim/Downloads/android-ndk-r8c
	alias ndk-build=/Users/sarim/Downloads/android-ndk-r8c/ndk-build
	export NDK_TOOLCHAIN_VERSION=clang3.1

(NB: I have a old ndk (r8c) so clang 3.1, latest one maybe have 3.3, be sure to check this)

Now compile:

	ndk-build -j 4


####Now goto eclipse and do what you want to do ;)


--


TODO
==

0. ~~`(Important) [ndk]` Make v8 with snapshot support~~
0. ~~`(Important) [ndk]` Fix android build script to generate libraries*cc files. They are generated using some python based script. (Now i'm just coping them from normal make)~~
0. `(Easy) [git]` Need to add stlport and v8 as git submodule.
0. `(Massive) [sdk]` Implement all functions of a input method.

If you are a android + unix + c geek, maybe help with these ? ^_^
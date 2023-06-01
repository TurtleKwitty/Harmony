# make it easy to run from vim
.PHONY: run

run: build
	@./build/Harmony

#It is cmake that ensure build short-circuiting so always run here
build: FORCE
	@cmake . -Bbuild
	@$(MAKE) -C ./build --no-print-directory

FORCE: ;

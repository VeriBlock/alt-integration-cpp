clean:
	rm -rf ../c/lib/libveriblock-pop-c.a ../c/lib/libveriblock-pop-cpp.a

build:
	mkdir -p ../../build && \
	cd ../../build && \
	cmake .. && \
	make veriblock-pop-c && \
	mv lib/libveriblock-pop-c.a lib/libveriblock-pop-cpp.a ../bindings/c/lib/

test: $(build)
	go test ./...

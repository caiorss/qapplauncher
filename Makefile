debug:
	cmake -B_build -H. -DCMAKE_BUILD_TYPE=Debug
	cmake --build _build --target

release:
	cmake -B_build -H. -DCMAKE_BUILD_TYPE=Release
	cmake --build _build --target

clean:
	rm -rfv ./_build ./bin/*

# Run: make debug or make release and then make run. 
run:
	bin/applauncher 

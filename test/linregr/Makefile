all: eigen armadillo plain plain-c

VECLEN = 100
NUMITER = 1000000

eigen: outerprod.cpp
	$(CXX) -O3 -DUSE_EIGEN -DEIGEN_NO_DEBUG -I eigen-3.0.3 $< -o $@

armadillo: outerprod.cpp
	$(CXX) -O3 -DUSE_ARMADILLO -DARMA_NO_DEBUG -I armadillo-1.2.0/include -I boost-1.46.1 -llapack -lblas $< -o $@

plain: outerprod.cpp
	$(CXX) -O3 $< -o $@

plain-c: outerprod.cpp
	$(CC) -O3 -x c -std=c99 $< -o $@

time: all
	@printf "Testing outer-product using Eigen...\n"
	time ./eigen $(VECLEN) $(NUMITER) > /dev/null
	@printf "\n\nTesting outer-product using Armadillo...\n"
	time ./armadillo ${VECLEN} ${NUMITER} > /dev/null
	@printf "\n\nTesting outer-product using a nested loop...\n"
	time ./plain ${VECLEN} ${NUMITER} > /dev/null
	@printf "\n\nTesting outer-product using a nested loop (compiled as C)...\n"
	time ./plain-c ${VECLEN} ${NUMITER} > /dev/null

clean:
	rm -rf eigen armadillo plain plain-c

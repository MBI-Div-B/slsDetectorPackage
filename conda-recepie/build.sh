
mkdir build
mkdir install
cd build
cmake .. \
      -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      -DCMAKE_INSTALL_PREFIX=install \
      -DSLS_USE_TEXTCLIENT=ON \
      -DSLS_USE_RECEIVER=ON \
      -DSLS_USE_GUI=ON \
      -DSLS_USE_TESTS=ON \
      -DSLS_USE_PYTHON=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -DSLS_USE_HDF5=OFF\
     
NCORES=$(getconf _NPROCESSORS_ONLN)
echo "Building using: ${NCORES} cores"
cmake --build . -- -j${NCORES}
cmake --build . --target install

CTEST_OUTPUT_ON_FAILURE=1 ctest -j 2

# ACG Final Project

Title: Octree-based Graph Representation for 3d Model Classification [[Paper](https://docs.google.com/document/d/1cGP5hOtmHJmcghXO9DlNz5IIxB0rwIbwy8E-H4Bds8Q/edit?usp=sharing)]

Please be advised that our project only works on Windows operating system

How to run:

1. Install all dependencies:

``` bash
pip install -r requirements.txt
```

2. Download dataset using Kaggle API:

``` bash
kaggle datasets download -d balraj98/modelnet40-princeton-3d-object-dataset
unzip modelnet40-princeton-3d-object-dataset.zip -d ../Dataset/ModelNet40
```

Or download manually from [Kaggle](https://www.kaggle.com/datasets/balraj98/modelnet40-princeton-3d-object-dataset) and place the data under `../Dataset/ModelNet40`.


3. To generate baked information directly, you should compile the CMake project in GraphGenerator. The command to run the generator is:

``` bash
./GraphGenerator.exe <file_path> <layer> [-p <pathgraph_raw_data_output>] [-b <adjacent_matrix_image_path>] [-r]
```

- -p Create pathgraph raw data
- -b Create adjacent matrix image
- -r Rotate the model to create rotation-invariant data

4. Run the following shell commands:

``` bash
python Bake.py
```

5. Run notebooks in the following order, change hyperparameters as desired:

- `data_processing.ipynb`
- `label_encoder.ipynb`
- `train-7.ipynb`
- `texture_generation.ipynb`

6. *Submitty Special Notes* Since there is a limitation of file size, the visualizer (an unity project for checking pathgraph) is not submitted to submitty repo. It can be found at [here](https://github.com/keran-w/3d-deep-learning/tree/main/Visualizer).

# ACG Final Project

Title: Octree-based Graph Representation for 3d Model Classification [[Paper](https://docs.google.com/document/d/1cGP5hOtmHJmcghXO9DlNz5IIxB0rwIbwy8E-H4Bds8Q/edit?usp=sharing)]

Please be advised that our project only works on Windows operating system

How to run:

1. Install all dependencies:

``` bash
pip install -r requirements.txt
```

2. download dataset using Kaggle API:

``` bash
kaggle datasets download -d balraj98/modelnet40-princeton-3d-object-dataset
unzip modelnet40-princeton-3d-object-dataset.zip -d ../Dataset/ModelNet40
```

Or download manually from [Kaggle](https://www.kaggle.com/datasets/balraj98/modelnet40-princeton-3d-object-dataset) and place the data under `../Dataset/ModelNet40`.

3. run the following shell commands:

``` bash
python Bake.py
```

4. Run notebooks in the following order, change hyperparameters as desired:

- `data_processing.ipynb`
- `train-7.ipynb`
- `texture_generation.ipynb`

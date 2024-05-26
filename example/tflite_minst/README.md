# minst
## how to use

## support chip
|support  chip|
|:----------- |
|CH32V307     |

## notice
Use `train/test.ipynb` to train a model that has been quantized by int8, and due to the large number of operators used, you need to enable the non-zero wait area of ch32v307, so we add `r128k_f480k` in the cmake file.
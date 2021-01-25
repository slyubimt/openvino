## MaxPool <a name="MaxPool"></a> {#openvino_docs_ops_pooling_MaxPool_1}

**Versioned name**: *MaxPool-1*

**Category**: *Pooling*

**Short description**: [Reference](http://caffe.berkeleyvision.org/tutorial/layers/pooling.html)

**Detailed description**: [Reference](http://cs231n.github.io/convolutional-networks/#pool)

**Attributes**: *Pooling* attributes are specified in the `data` node, which is a child of the layer node.

* *strides*

  * **Description**: *strides* is a distance (in pixels) to slide the window on the feature map over the (z, y, x) axes for 3D poolings and (y, x) axes for 2D poolings. For example, *strides* equal "4,2,1" means sliding the window 4 pixel at a time over depth dimension, 2 over height dimension and 1 over width dimension.
  * **Range of values**: integer values starting from 0
  * **Type**: int[]
  * **Default value**: None
  * **Required**: *yes*

* *pads_begin*

  * **Description**: *pads_begin* is a number of pixels to add to the beginning along each axis. For example, *pads_begin* equal "1,2" means adding 1 pixel to the top of the input and 2 to the left of the input.
  * **Range of values**: integer values starting from 0
  * **Type**: int[]
  * **Default value**: None
  * **Required**: *yes*
  * **Note**: the attribute is ignored when *auto_pad* attribute is specified.

* *pads_end*

  * **Description**: *pads_end* is a number of pixels to add to the ending along each axis. For example, *pads_end* equal "1,2" means adding 1 pixel to the bottom of the input and 2 to the right of the input.
  * **Range of values**: integer values starting from 0
  * **Type**: int[]
  * **Default value**: None
  * **Required**: *yes*
  * **Note**: the attribute is ignored when *auto_pad* attribute is specified.

* *kernel*

  * **Description**: *kernel* is a size of each filter. For example, *kernel* equal (2, 3) means that each filter has height equal to 2 and width equal to 3.
  * **Range of values**: integer values starting from 1
  * **Type**: int[]
  * **Default value**: None
  * **Required**: *yes*

* *rounding_type*

  * **Description**: *rounding_type* is a type of rounding to be applied.
  * **Range of values**:
    * *ceil*
    * *floor*
  * **Type**: string
  * **Default value**: *floor*
  * **Required**: *no*

* *auto_pad*

  * **Description**: *auto_pad* how the padding is calculated. Possible values:
    * *explicit*: use explicit padding values from `pads_begin` and `pads_end`.
    * *same_upper (same_lower)* the input is padded to match the output size. In case of odd padding value an extra padding is added at the end (at the beginning).
    * *valid* - do not use padding.
  * **Type**: string
  * **Default value**: *explicit*
  * **Required**: *no*
  * **Note**: *pads_begin* and *pads_end* attributes are ignored when *auto_pad* is specified.

**Inputs**:

*   **1**: 3D, 4D or 5D input tensor. Required.

**Mathematical Formulation**

    \f[
    output_{j} = max(x_{0}, ..., x_{i})
    \f]

**Example**

```xml
<layer ... type="MaxPool" ... >
        <data auto_pad="same_upper" kernel="3,3" pads_begin="0,0" pads_end="1,1" strides="2,2"/>
        <input> ... </input>
        <output> ... </output>
</layer>
```
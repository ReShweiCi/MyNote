# LearnOpenGL CN 入门-纹理 2023.09.19
## 纹理坐标系
坐标系左下角为(0,0),右上角为(1,1)

![](./Pictures/tex_coords.png)

为了在三角形上绘制纹理，将三角形左下角顶点的纹理坐标设为(0,0),上顶点纹理坐标对应为(0.5, 1),右下角对应为(1.0, 0),代码如下
```glsl
float texCoords[] = {
    0.0f, 0.0f, // 左下角
    1.0f, 0.0f, // 右下角
    0.5f, 1.0f // 上中
};
```
## 纹理环绕方式
当纹理坐标被设置在(1, 1)之外时，OpenGL会重复绘制这个纹理，环绕方式如下

![](./Pictures/texture_wrapping.png)

| 环绕方式 | 描述 |
| :----: | :----: | 
| <b>GL_REPEAT</b> | 对纹理的默认行为。重复纹理图像。|
| <b>GL_MIRRORED_REPEAT</b> | 和GL_REPEAT一样，但每次重复图片是镜像放置的。 |
| <b>GL_CLAMP_TO_EDGE</b> | 纹理坐标会被约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果。 |
| <b>GL_CLAMP_TO_BORDER</b> | 超出的坐标为用户指定的边缘颜色。 |

可以使用`glTexParameter*`为每个坐标轴单独设置纹理环绕方式(s, t, r或x, y, z)，如
```glsl
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
```

第一个参数指定了<b>纹理目标</b>；我们使用的是2D纹理，因此纹理目标是`GL_TEXTURE_2D`。第二个参数需要我们指定设置的选项与应用的<b>纹理轴</b>。我们打算配置的是`WRAP`选项，并且指定S和T轴。最后一个参数需要我们传递一个环绕方式(Wrapping)，在这个例子中OpenGL会给当前激活的纹理设定纹理环绕方式为`GL_MIRRORED_REPEAT`。

如果我们选择`GL_CLAMP_TO_BORDER`选项，我们还需要指定一个边缘的颜色。这需要使用`glTexParameter`函数的fv后缀形式，用`GL_TEXTURE_BORDER_COLOR`作为它的选项，并且传递一个float数组作为边缘的颜色值：
```glsl
float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
```
## 纹理过滤
将int类型的纹理像素映射到float类型的纹理坐标时，需要进行纹理过滤，在将低分辨率纹理映射到大物体上时尤为明显，其中最重要的两种过滤方式为<b>邻近过滤</b>(OpenGL默认)`GL_NEAREST`和<b>线性过滤</b>`GL_LINEAR`

![](./Pictures/texture_filtering.png)

通过如下方法进行设置放大和缩小纹理时的过滤方式
```glsl
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```
## 多级渐远纹理(Mipmap)
当物体离摄像机很远时，采用高分辨率纹理不仅会失真，因为需要从很大一块纹理上拾取出一小块像素，还会带来资源浪费的问题，Mipmap则可以解决这个问题

![](./Pictures/mipmaps.png)

Mipmap也有以下过滤方式
| 过滤方式 | 描述 |
| :----: | :----: |
| <b>GL_NEAREST_MIPMAP_NEAREST</b> | 使用最邻近的多级渐远纹理来匹配像素大小，并使用邻近插值进行纹理采样 |
| <b>GL_LINEAR_MIPMAP_NEAREST</b> | 使用最邻近的多级渐远纹理级别，并使用线性插值进行采样 |
| <b>GL_NEAREST_MIPMAP_LINEAR</b> | 在两个最匹配像素大小的多级渐远纹理之间进行线性插值，使用邻近插值进行采样 |
| <b>GL_LINEAR_MIPMAP_LINEAR</b> | 在两个邻近的多级渐远纹理之间使用线性插值，并使用线性插值进行采样 |

我们同样使用`glTexParameteri`将过滤方式设置为四种方法之一
```glsl
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
```
<b>注意，将放大过滤的选项设置为多级渐远纹理过滤选项之一没有任何效果，会产生`GL_INVALID_ENUM`错误代码</b>
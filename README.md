# Renju-UCAS-C-programming

中国科学院大学本科程序设计基础与实验（C语言）期末大作业：五子棋（部分禁手）AI的实现（2022级杨力祥班比赛第一名）

**说明**
1. 文件采用GBK编码保存，使用GBK编码才能看到正确的注释！
2. 本程序不使用旧版控制台！如果必须使用旧版控制台，应做如下修改：
- 把`print_with_color`函数改为`printf`（直接修改`io.c`中的`print_with_color`函数即可）
- 把`io.c`中的`display`函数中棋盘多余的横线去掉（如，把“┬─”改为“┬”）以显示正常的棋盘
- 如还有其他问题可以简单修改`display`函数以实现效果
3. 本程序使用了`windows.h`，利用Win32 API实现多线程，必须在Windows系统上编译运行！
4. 程序框架图见`Structure.jpg`，图中的箭头仅表示逻辑上的依赖关系，不表示代码的引用关系。
5. 编译本程序时，最好用较高版本的编译器：
- 为了适应VS 2022必须使用`_getch()`函数代替`getch()`函数，在VS 2017以前的版本中应换为`getch()`函数
- VS开发环境自动在每个头文件前增加`#pragma once`，这在低版本的编译器中可能不支持
6. 如果编译器对`scanf`报错，请在项目项目选项中的预处理器命令中添加`#define _CRT_SECURE_NO_WARNINGS`，或手动在每个文件前添加
7. 为了提高走子速度，建议使用-O2优化指令。
8. 由于时间问题，没有实现对保存的棋盘文件进行复盘的操作。但是，保存的棋盘文件的格式按照常用的[YIXIN BOARD](https://www.aiexp.info/pages/yixin.html)的保存格式，因此可以利用YIXIN BOARD打开
9. 程序创作过程花费大约1个半月，主要是每周六上午写一点。在开始创作程序之前，使用了Python测试了Min-Max(alpha-beta)搜索算法和朴素的MCTS算法，最终决定选择Min-Max(alpha-beta)搜索算法
10. 在禁手实现上，本程序不考虑十分复杂的禁手（忽略禁手之间的相互影响），这能够应对大部分情况
11. 程序创作过程中，主要参考：
- [GitHub上某大佬使用JS的五子棋教程](https://github.com/lihongxun945/myblog/labels/五子棋AI教程第二版)，整体的思路参考了这个教程，但在搜索方面我采用了先用较低层的搜索树进行预搜索再用较深层的搜索树进行深度搜索的方式，这是与该教程最大的不同
- [2020级武成岗班第一名程序](https://github.com/MingZwhy/UCAS-C_programming)

在此表示感谢！

一些碎碎念：

> 本人并非计算机相关专业的学生，想到此后不一定再有机会去尝试用C语言去从零开始写一个不算小的项目，遂决定将最终的成果上传到网络。
>
> 这样做一来是纪念这一个学期的努力，二则是为后来者提供一定的参考（我的程序整体上有一定的可读性）。
>
> 回想起最初面对这样一个大工程时的手足无措，到后来查阅了不少论文资料并经过一定的“预实验”选择了最终的思路，再到最终经过一个月多的时间反复修改重构调整最终在比赛当天交上了作业，这期间遇到了许多困难，但最终都一一克服。这一整个过程中，虽然我的专业知识并没有特别多的增长，但个人解决问题和结构思维的能力却得到了不小的锻炼。
>
> 我想感谢许多人：
>
> 首先感谢杨力祥老师提供这样一个难得的机会。我向来喜欢挑战一些困难的问题，完成这项大作业的过程中的一次次失败后的突破都让我感到欣喜与激动。
>
> 其次感谢我高中信息竞赛的教练赵老师，虽然只在您的指导下学习了一年多，但我在图书馆或宿舍敲代码时总能想起当时在机房备赛的快乐的时光。
>
> 最后，感谢自己，没有因为这件事情不重要就随便对付一下，而是仍以十分的热忱去应对。



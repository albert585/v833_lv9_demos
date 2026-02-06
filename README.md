# v833_lv9_demos
全志v833主控基于Tina Linux运行LVGL实例

使用LVGL9.4

# 使用方法

## 1. 拉取项目：
```bash
git clone --recursive https://github.com/albert585/v833_lv9_demos
```

## 2. 编译项目：
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake -B build -S .
make -C build -j$(nproc)
```

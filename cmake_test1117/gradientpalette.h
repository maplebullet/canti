#ifndef GRADIENTPALETTE_H
#define GRADIENTPALETTE_H

#include <QLinearGradient>

class GradientPalette
{
public:
    GradientPalette(int width);
    ~GradientPalette();

    void setColorAt(qreal index, const QColor &color);
    QColor getColorAt(qreal index);


private:
    // 作为调色板的画布
    QImage *canvas_;
    // 线性渐变
    QLinearGradient gradient_;
    // 宽度
    int width_;
};

#endif // GRADIENTPALETTE_H

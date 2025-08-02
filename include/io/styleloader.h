#include <QString>
#include <QWidget>

namespace IO {
    class StyleLoader{
    public:
        static QString loadStyle(const QString& resourcePath);
        static void applyStyle(QWidget* widget, const QString& resourcePath);
    };
} // namespace IO

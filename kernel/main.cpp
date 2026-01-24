#include "geometry/geometry.h"
#include "screen.h"
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdlib>
int main(int argc, char *argv[]) {
  srand(228);
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("Генератор дендизма");

  auto *layout = new QVBoxLayout(&window);

  auto *imageLabel = new QLabel();
  imageLabel->setAlignment(Qt::AlignCenter);

  auto *button = new QPushButton("стать денди");

  layout->addWidget(imageLabel);
  layout->addWidget(button);

  QObject::connect(button, &QPushButton::clicked, [&]() {
    QImage img = detail::GenerateTriangle();
    imageLabel->setPixmap(QPixmap::fromImage(img));
  });

  window.show();

  return app.exec();
}

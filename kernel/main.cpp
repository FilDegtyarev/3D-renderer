#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "screen/screen.h"
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdlib>

#include <iostream>
int main(int argc, char *argv[]) {
  srand(1329);
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("Треугольники");

  auto *layout = new QVBoxLayout(&window);

  detail::screen::Screen screen(detail::screen::Height(500),
                                detail::screen::Width(500));
  screen.Connect(layout);

  auto *button = new QPushButton("еще треугольники");

  layout->addWidget(button);

  QObject::connect(button, &QPushButton::clicked, [&]() {
    QImage img = detail::rasterization::GenerateRandomTrinagleFilled();
    // imageLabel->setPixmap(QPixmap::fromImage(img));
    screen.Update(img);
  });

  window.show();

  return app.exec();
}

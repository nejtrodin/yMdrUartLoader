#ifndef FILEIOHELPER_H
#define FILEIOHELPER_H

#include <QObject>

class FileIoHelper : public QObject
{
  Q_OBJECT
public:
  explicit FileIoHelper(QObject *parent = nullptr);

  Q_INVOKABLE void writeTextFile(const QString filePath, const QString &text);
};

#endif // FILEIOHELPER_H

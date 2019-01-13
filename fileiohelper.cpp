#include "fileiohelper.h"
#include <QFile>
#include <QTextStream>

FileIoHelper::FileIoHelper(QObject *parent) :
  QObject (parent)
{

}

void FileIoHelper::writeTextFile(const QString filePath, const QString &text)
{
  QFile file(filePath);
  if (!file.open(QFile::WriteOnly))
    return;

  QTextStream stream(&file);
  stream << text;

  file.close();
}

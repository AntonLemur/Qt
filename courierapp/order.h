#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QDateTime>

struct Order {
    int Id=0;
    QString Address="";
    QString Customer="";
    QDateTime OrderDate; // Order_Date.isValid() = false, "1970-01-01T00:00:00"
    double Amount=0.0;
    int Status=0;
};

#endif // ORDER_H

/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdltfilterlist.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>
#include <QMessageBox>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltFilterList::QDltFilterList()
{

}

QDltFilterList::~QDltFilterList()
{
    clearFilter();
}

QDltFilterList& QDltFilterList::operator= (QDltFilterList const& _filterList)
{
    QDltFilter *filter_source,*filter_copy;
    clearFilter();
    for(int numfilter=0;numfilter<_filterList.filters.size();numfilter++)
    {
        filter_copy = new QDltFilter();
        filter_source = filters[numfilter];
        *filter_copy = *filter_source;
        filters.append(filter_copy);
    }

    return *this;
}

void QDltFilterList::clearFilter()
{
    QDltFilter *filter;

    for(int numfilter=0;numfilter<filters.size();numfilter++)
    {
        filter = filters[numfilter];
        delete filter;
    }
    filters.clear();
    qDebug() << "clearFilter: Clear filter";
}

void QDltFilterList::addFilter(QDltFilter *_filter)
{
    filters.append(_filter);
    qDebug() << "addFilter: Add Filter" << _filter->apid << _filter->ctid;
}

QColor QDltFilterList::checkMarker(QDltMsg &msg)
{
    QDltFilter *filter;
    QColor color;

    for(int numfilter=0;numfilter<filters.size();numfilter++)
    {
        filter = filters[numfilter];

        if(filter->isMarker() && filter->enableFilter)
        {
            if(filter->match(msg))
            {
                color = filter->filterColour;
                break;
            }
        }
    }
    return color;
}

bool QDltFilterList::checkFilter(QDltMsg &msg)
{
    QDltFilter *filter;
    bool found = false;
    bool filterActivated = false;

    /* If there are no positive filters, or all positive filters
     * are disabled, the default case is to show all messages. Only
     * negative filters will be applied */
    for(int numfilter=0;numfilter<filters.size();numfilter++)
    {
        filter = filters[numfilter];
        if(filter->isPositive() && filter->enableFilter){
            filterActivated = true;
        }
    }

    if(filterActivated==false)
        found = true;
    else
        found = false;


    for(int numfilter=0;numfilter<filters.size();numfilter++)
    {
        filter = filters[numfilter];
        if(filter->isPositive() && filter->enableFilter) {
            found = filter->match(msg);
            if (found)
              break;
        }
    }

    if (found || filterActivated==false ){
        //we need only to check for negative filters, if the message would be shown! If discarded anyway, there is no need to apply it.
        //if positive filter applied -> check for negative filters
        //if no positive filters are active or no one exists, we need also to filter negatively
        // if the message has been discarded by all positive filters before, we do not need to filter it away a second time

        for(int numfilter=0;numfilter<filters.size();numfilter++)
          {
            filter = filters[numfilter];
            if(filter->isNegative() && filter->enableFilter){
                if (filter->match(msg))
                  {
                    // a negative filter has matched -> found = false
                    found = false;
                    break;
                  }
              }
          }
      }

    return found;
}

bool QDltFilterList::SaveFilter(QString _filename)
{
    QFile file(_filename);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
            QMessageBox::critical(0, QString("DLT Viewer"),QString("Save DLT Filter file failed!"));
            return false;
    }

    filename = _filename;

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("dltfilter");


    /* Write Filter */
    for(int num = 0; num < filters.size(); num++)
    {
        QDltFilter *filter = filters[num];

        xml.writeStartElement("filter");
        filter->SaveFilterItem(xml);

        xml.writeEndElement(); // filter
    }

    xml.writeEndElement(); // dltfilter
    xml.writeEndDocument();

    file.close();

    return true;
}

bool QDltFilterList::LoadFilter(QString _filename, bool replace){

    QFile file(_filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(0, QString("DLT Viewer"),QString("Loading DLT Filter file failed!"));
        return false;
    }

    filename = _filename;

    QDltFilter filter;

    if(replace)
        filters.clear();

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {

              if(xml.name() == QString("filter"))
              {
                  filter.clear();
              }
              filter.LoadFilterItem(xml);
          }
          if(xml.isEndElement())
          {
              if(xml.name() == QString("filter"))
              {
                    QDltFilter *filter_new = new QDltFilter();
                    *filter_new = filter;
                    filters.append(filter_new);
              }

          }
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, QString("XML Parser error"),
                             xml.errorString());
    }

    file.close();

    return true;
}
#include "customerdatasystem.h"

#include <ve_eventdata.h>
#include <ve_commandevent.h>
#include <ve_storagesystem.h>
#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_errordata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_remoteproceduredata.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSaveFile>
#include <QFile>
#include <QtConcurrent>
#include <QRegularExpressionValidator>

#include <functional>

using namespace VeinComponent;
using namespace VfCustomerdata;






CustomerDataSystem::CustomerDataSystem(QString p_customerDataPath, int p_entityId, QObject *t_parent) :
    QObject(t_parent),
    m_customerDataPath(p_customerDataPath)
{
    Q_ASSERT(QString(m_customerDataPath).isEmpty() == false);
    QFileInfo cDataFileInfo(m_customerDataPath);
    if(cDataFileInfo.exists() && cDataFileInfo.isDir() == false) {
        qWarning() << "Invalid path to customer data, file is not a directory:" << m_customerDataPath;
    }
    else if(cDataFileInfo.exists() == false) {
        QDir customerdataDir;
        qWarning() << "Customer data directory does not exist:" << m_customerDataPath;
        if(customerdataDir.mkdir(m_customerDataPath) == false) {
            qWarning() << "Could not create customerdata direcory:" << m_customerDataPath;
        }
    }

    m_entity=VfCpp::VeinModuleEntity::Ptr(new VfCpp::VeinModuleEntity(p_entityId));

    QObject::connect(m_entity.data() ,&VeinEvent::EventSystem::sigAttached,this,&CustomerDataSystem::initOnce);
}


void CustomerDataSystem::initOnce()
{
    QJsonDocument introspectionDoc;
    QJsonObject introspectionRootObject;
    QJsonObject componentInfo;
    QJsonObject remoteProcedureInfo;


    m_entity->createComponent("EntityName","CustomerData",VfCpp::cVeinModuleComponent::Direction::constant);


    m_entity->createRpc(this,"RPC_Open",{{"p_fileName", "QString"}});
    m_entity->createRpc(this,"RPC_Close",{{"p_save", "bool"}});
    m_entity->createRpc(this,"RPC_Save",{});

    m_fileSelected=m_entity->createComponent("FileSelected",QString(""),VfCpp::cVeinModuleComponent::Direction::out);
    QRegularExpression rx(".*\\.json");
    static_cast<VfCpp::cVeinModuleComponent*>(m_fileSelected.component().toStrongRef().data())->setValidator(new QRegularExpressionValidator(rx));

    m_fileEntrieComponents["PAR_DatasetIdentifier"]= m_entity->createComponent("PAR_DatasetIdentifier",QString(""));
    m_fileEntrieComponents["PAR_DatasetComment"]=m_entity->createComponent("PAR_DatasetComment",QString(""));
    m_fileEntrieComponents["PAR_CustomerFirstName"]=m_entity->createComponent("PAR_CustomerFirstName",QString(""));
    m_fileEntrieComponents["PAR_CustomerLastName"]=m_entity->createComponent("PAR_CustomerLastName",QString(""));
    m_fileEntrieComponents["PAR_CustomerStreet"]=m_entity->createComponent("PAR_CustomerStreet",QString(""));
    m_fileEntrieComponents["PAR_CustomerPostalCode"]=m_entity->createComponent("PAR_CustomerPostalCode",QString(""));
    m_fileEntrieComponents["PAR_CustomerCity"]=m_entity->createComponent("PAR_CustomerCity",QString(""));
    m_fileEntrieComponents["PAR_CustomerCountry"]=m_entity->createComponent("PAR_CustomerCountry",QString(""));
    m_fileEntrieComponents["PAR_CustomerNumber"]=m_entity->createComponent("PAR_CustomerNumber",QString(""));
    m_fileEntrieComponents["PAR_CustomerComment"]=m_entity->createComponent("PAR_CustomerComment",QString(""));

    m_fileEntrieComponents["PAR_LocationFirstName"]=m_entity->createComponent("PAR_LocationFirstName",QString(""));
    m_fileEntrieComponents["PAR_LocationLastName"]=m_entity->createComponent("PAR_LocationLastName",QString(""));
    m_fileEntrieComponents["PAR_LocationStreet"]=m_entity->createComponent("PAR_LocationStreet",QString(""));
    m_fileEntrieComponents["PAR_LocationPostalCode"]=m_entity->createComponent("PAR_LocationPostalCode",QString(""));
    m_fileEntrieComponents["PAR_LocationCity"]=m_entity->createComponent("PAR_LocationCity",QString(""));
    m_fileEntrieComponents["PAR_LocationCountry"]=m_entity->createComponent("PAR_LocationCountry",QString(""));
    m_fileEntrieComponents["PAR_LocationNumber"]=m_entity->createComponent("PAR_LocationNumber",QString(""));
    m_fileEntrieComponents["PAR_LocationComment"]=m_entity->createComponent("PAR_LocationComment",QString(""));

    m_fileEntrieComponents["PAR_PowerGridOperator"]=m_entity->createComponent("PAR_PowerGridOperator",QString(""));
    m_fileEntrieComponents["PAR_PowerGridSupplier"]=m_entity->createComponent("PAR_PowerGridSupplier",QString(""));
    m_fileEntrieComponents["PAR_PowerGridComment"]=m_entity->createComponent("PAR_PowerGridComment",QString(""));

    m_fileEntrieComponents["PAR_MeterManufacturer"]=m_entity->createComponent("PAR_MeterManufacturer",QString(""));
    m_fileEntrieComponents["PAR_MeterFactoryNumber"]=m_entity->createComponent("PAR_MeterFactoryNumber",QString(""));
    m_fileEntrieComponents["PAR_MeterOwner"]=m_entity->createComponent("PAR_MeterOwner",QString(""));
    m_fileEntrieComponents["PAR_MeterComment"]=m_entity->createComponent("PAR_MeterComment",QString(""));

}


QVariant CustomerDataSystem::RPC_Open(QVariantMap p_params)
{
    QString fileName=p_params["p_fileName"].toString();
    QFile customerDataFile(QString("%1%2").arg(m_customerDataPath).arg(fileName));

    QJsonParseError parseError;

    // create file if file does not exist
    if(!customerDataFile.exists()){
        customerDataFile.open(QFile::WriteOnly);
        QJsonDocument dataDocument;
        QJsonObject rootObject;
        for(const QString &entryName : m_fileEntrieComponents.keys()) {
            rootObject.insert(entryName, QString());
        }
        dataDocument.setObject(rootObject);
        customerDataFile.write(dataDocument.toJson(QJsonDocument::Indented));
        customerDataFile.close();
    }

    //open File read data and close it. We do that because we do not want to keep it open the entire time
    //and it makes no difference for the user.
    if(customerDataFile.open(QFile::ReadOnly)) {
        m_currentCustomerDocument = QJsonDocument::fromJson(customerDataFile.readAll(), &parseError);

         if(m_currentCustomerDocument.isObject()) {
            const QJsonObject tmpObject = m_currentCustomerDocument.object();
            RPC_Close(QVariantMap());
            // set filename
            m_fileSelected=fileName;
            // read all docuement entries
            for(QString componentName : tmpObject.keys()) {
                if(m_fileEntrieComponents.contains(componentName)){
                    m_fileEntrieComponents[componentName]=tmpObject.value(componentName).toString();
                }
            }
            customerDataFile.close();
            return true;

         }

    }
    return false;
}

QVariant CustomerDataSystem::RPC_Close(QVariantMap p_params)
{
    Q_UNUSED(p_params);
    bool save=p_params["p_save"].toBool();
    if(save == true){
        RPC_Save(QVariantMap());
    }
    m_fileSelected="";
    for(VfCpp::VeinSharedComp<QString> component : m_fileEntrieComponents.values()) {
        component.setValue("");
    }
    m_currentCustomerDocument.setObject(QJsonObject());
    return true;
}


QVariant CustomerDataSystem::RPC_Save(QVariantMap p_params)
{
    Q_UNUSED(p_params);
    if(!m_fileSelected.value().isEmpty()){
        QSaveFile customerDataFile(QString("%1%2").arg(m_customerDataPath).arg(m_fileSelected.value()));
        QJsonObject tmpObject = m_currentCustomerDocument.object();
        for(QString key : m_fileEntrieComponents.keys()){
            tmpObject.insert(key,m_fileEntrieComponents[key].value());
        }
        customerDataFile.open(QIODevice::WriteOnly);
        m_currentCustomerDocument.setObject(tmpObject);
        customerDataFile.write(m_currentCustomerDocument.toJson(QJsonDocument::Indented));
        if(customerDataFile.commit() == true) {
            return true;
        }else{
            qCritical() << "Error writing customerdata json file:" << m_fileSelected.value();
        }
    }
    return false;
}


VfCpp::VeinModuleEntity* CustomerDataSystem::entity() const
{
    return m_entity.data();
}

void CustomerDataSystem::setEntity(VfCpp::VeinModuleEntity* entity)
{
    m_entity = VfCpp::VeinModuleEntity::Ptr(entity);
}




//<LICENSE>

#include "niqaconfig.h"

#include <iomanip>
#include <strings/miscstring.h>
#include <strings/string2array.h>
#include <base/KiplException.h>

#include <QDebug>

NIQAConfig::NIQAConfig() :
    logger("NIQAConfig")
{

}

NIQAConfig::NIQAConfig(NIQAConfig &c) :
    logger("NIQAConfig"),
    userInformation(c.userInformation),
    contrastAnalysis(c.contrastAnalysis),
    edgeAnalysis2D(c.edgeAnalysis2D),
    edgeAnalysis3D(c.edgeAnalysis3D),
    ballPackingAnalysis(c.ballPackingAnalysis)
{
}

const NIQAConfig & NIQAConfig::operator=(const NIQAConfig &c)
{
    userInformation     = c.userInformation;
    contrastAnalysis    = c.contrastAnalysis;
    edgeAnalysis2D      = c.edgeAnalysis2D;
    edgeAnalysis3D      = c.edgeAnalysis3D;
    ballPackingAnalysis = c.ballPackingAnalysis;

    return *this;
}

const std::string NIQAConfig::WriteXML()
{
    std::ostringstream str;

    str<<"<niqa>\n";
    str<<userInformation.WriteXML(4);
    str<<contrastAnalysis.WriteXML(4);
    str<<edgeAnalysis2D.WriteXML(4);
    str<<edgeAnalysis3D.WriteXML(4);
    str<<ballPackingAnalysis.WriteXML(4);
    str<<"</niqa>\n";

    return str.str();
}

void NIQAConfig::loadConfigFile(std::string configfile, std::string ProjectName)
{
    xmlTextReaderPtr reader;
    const xmlChar *name;
    int ret;
    std::string sName;
    std::ostringstream msg;

    reader = xmlReaderForFile(configfile.c_str(), nullptr, 0);
    if (reader != nullptr) {
        ret = xmlTextReaderRead(reader);
        name = xmlTextReaderConstName(reader);


        if (name==nullptr) {
            throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
        }

        sName=reinterpret_cast<const char *>(name);
        msg.str(""); msg<<"Found "<<sName<<" expect "<<ProjectName;
        logger(kipl::logging::Logger::LogMessage,msg.str());
        if (std::string(sName)!=ProjectName) {
            msg.str();
            msg<<"Unexpected project contents in parameter file ("<<sName<<"!="<<ProjectName<<")";
            logger(kipl::logging::Logger::LogMessage,msg.str());
            throw kipl::base::KiplException(msg.str(),__FILE__,__LINE__);
        }

        logger(kipl::logging::Logger::LogVerbose,"Got project");

        ret = xmlTextReaderRead(reader);

        while (ret == 1) {
            if (xmlTextReaderNodeType(reader)==1) {
                name = xmlTextReaderConstName(reader);

                if (name==nullptr) {
                    throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
                }
                sName=reinterpret_cast<const char *>(name);

                parseConfig(reader,sName);
            }
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            std::stringstream str;
            str<<"Module config failed to parse "<<configfile;
            throw kipl::base::KiplException(str.str(),__FILE__,__LINE__);
        }
    } else {
        std::stringstream str;
        str<<"Module config could not open "<<configfile;
        throw kipl::base::KiplException(str.str(),__FILE__,__LINE__);
    }
    logger(kipl::logging::Logger::LogVerbose,"Parsing parameter file done");
}

void NIQAConfig::parseConfig(xmlTextReaderPtr reader, std::string sName)
{

    if (sName=="userinformation")
        parseUserInformation(reader);

    if (sName=="contrastanalysis")
        ParseContrastAnalysis(reader);

    if (sName=="edgeanalysis2d")
        parseEdgeAnalysis2D(reader);

    if (sName=="edgeanalysis3d")
        parseEdgeAnalysis3D(reader);

    if (sName=="ballpacking")
        parseBallPackingAnalysis(reader);
}

void NIQAConfig::parseUserInformation(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    int ret = xmlTextReaderRead(reader);
    std::string sName, sValue;
    int depth=xmlTextReaderDepth(reader);

    while (ret == 1) {
        if (xmlTextReaderNodeType(reader)==1) {
            name = xmlTextReaderConstName(reader);
            ret=xmlTextReaderRead(reader);

            value = xmlTextReaderConstValue(reader);
            if (name==nullptr) {
                logger.error("Unexpected contents in parameter file");
                throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
            }
            if (value!=nullptr)
                sValue=reinterpret_cast<const char *>(value);
            else
                sValue="Empty";
            sName=reinterpret_cast<const char *>(name);

            if (sName=="operator") {
                userInformation.userName=sValue;
            }

            if (sName=="institute") {
                userInformation.institute=sValue;
            }

            if (sName=="instrument") {
                userInformation.instrument=sValue;
            }
            if (sName=="country") {
                userInformation.country=sValue;
            }
            if (sName=="experimentdate") {
                kipl::strings::String2Array(std::string(sValue),userInformation.experimentDate,3);
            }

            if (sName=="analysisdate") {
                kipl::strings::String2Array(std::string(sValue),userInformation.analysisDate,3);
            }

            if (sName=="version") {
                if (sValue!=std::string(VERSION))
                    logger.warning("Software version missmatch");
                userInformation.softwareVersion=VERSION;
            }

            if (sName=="comment") {
                userInformation.comment=sValue;
            }

            if (sName=="reportname") {
                userInformation.reportName=sValue;
            }
        }
        ret = xmlTextReaderRead(reader);
        if (xmlTextReaderDepth(reader)<depth)
            ret=0;
    }
}

void NIQAConfig::ParseContrastAnalysis(xmlTextReaderPtr reader)
{

    const xmlChar *name, *value;
    int ret = xmlTextReaderRead(reader);
    std::string sName, sValue;
    int depth=xmlTextReaderDepth(reader);

    while (ret == 1) {
        if (xmlTextReaderNodeType(reader)==1) {
            name = xmlTextReaderConstName(reader);
            ret=xmlTextReaderRead(reader);

            value = xmlTextReaderConstValue(reader);
            if (name==nullptr) {
                logger.error("Unexpected contents in parameter file");
                throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
            }
            if (value!=nullptr)
                sValue=reinterpret_cast<const char *>(value);
            else
                sValue="Empty";
            sName=reinterpret_cast<const char *>(name);

            if (sName=="filemask") {
                contrastAnalysis.fileMask=sValue;
            }

            if (sName=="first") {
                contrastAnalysis.first=std::stoi(sValue);
            }

            if (sName=="last") {
                contrastAnalysis.last=std::stoi(sValue);
            }

            if (sName=="step") {
                contrastAnalysis.step=std::stoi(sValue);
            }

            if (sName=="pixelsize") {
                contrastAnalysis.pixelSize= std::stod(sValue);
            }

            if (sName=="intensityslope") {
                contrastAnalysis.intensitySlope= std::stod(sValue);
            }

            if (sName=="intensityintercept") {
                contrastAnalysis.intensityIntercept = std::stod(sValue);
            }

            if (sName=="intensitymin") {
                contrastAnalysis.intensityMin= std::stod(sValue);
            }

            if (sName=="intensitymax") {
                contrastAnalysis.intensityMax = std::stod(sValue);
            }

            if (sName=="roilist") {
                std::vector<std::string> strlist;
                kipl::strings::String2Array(sValue,strlist);
                std::ostringstream msg;
                contrastAnalysis.analysisROIs.clear();

                msg<<"strlist.size: "<<strlist.size()<<", sValue: "<<sValue;
                logger.message(msg.str());
                if ((strlist.size() % 5 != 0) || (strlist.empty()==true)){
                    logger.error("Incomplete roi list");
                    throw kipl::base::KiplException("Incomplete ROI list",__FILE__,__LINE__);
                }

               // for (auto it=strlist.begin(); it!=strlist.end(); ++it) {
                for (size_t i=0; i<strlist.size(); i+=5)
                {

                    std::string lbl = strlist[i];
                    qDebug() << lbl.c_str();
                    std::vector<size_t> temproi = { std::stoul(strlist[i+1]),
                                                    std::stoul(strlist[i+2]),
                                                    std::stoul(strlist[i+3]),
                                                    std::stoul(strlist[i+4])};
                    kipl::base::RectROI roi(temproi,lbl);
                    contrastAnalysis.analysisROIs.push_back(roi);
                }

            }

            if (sName=="makereport") {
                contrastAnalysis.makeReport = kipl::strings::string2bool(sValue);
            }

        }
        ret = xmlTextReaderRead(reader);
        if (xmlTextReaderDepth(reader)<depth)
            ret=0;
    }
}

void NIQAConfig::parseEdgeAnalysis2D(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    int ret = xmlTextReaderRead(reader);
    std::string sName, sValue;
    int depth=xmlTextReaderDepth(reader);

    while (ret == 1) {
        if (xmlTextReaderNodeType(reader)==1) {
            name = xmlTextReaderConstName(reader);
            ret=xmlTextReaderRead(reader);

            value = xmlTextReaderConstValue(reader);
            if (name==nullptr) {
                logger.error("Unexpected contents in parameter file");
                throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
            }
            if (value!=nullptr)
                sValue=reinterpret_cast<const char *>(value);
            else
                sValue="Empty";
            sName=reinterpret_cast<const char *>(name);

            if (sName=="fitfunction") {
                edgeAnalysis2D.fitFunction=std::stoi(sValue);
            }

            if (sName=="multiimagelist"){
                std::list<std::string> sl;
                kipl::strings::String2List(sValue,sl);
                for (auto it=sl.begin(); it!=sl.end(); ++it) {
                    float pos=std::stof(*it); ++it;
                    std::string &fname=*it;
                    edgeAnalysis2D.multiImageList.insert(std::make_pair(pos,fname));
                }
            }

            if (sName=="obmask") {
                edgeAnalysis2D.obMask=sValue;
            }

            if (sName=="obfirst") {
                edgeAnalysis2D.obFirst=std::stoi(sValue);
            }

            if (sName=="oblast") {
                edgeAnalysis2D.obLast=std::stoi(sValue);
            }

            if (sName=="obstep") {
                edgeAnalysis2D.obStep=std::stoi(sValue);
            }

            if (sName=="dcmask") {
                edgeAnalysis2D.dcMask=sValue;
            }

            if (sName=="dcfirst") {
                edgeAnalysis2D.dcFirst=std::stoi(sValue);
            }

            if (sName=="dclast") {
                edgeAnalysis2D.dcLast=std::stoi(sValue);
            }

            if (sName=="dcstep") {
                edgeAnalysis2D.dcStep=std::stoi(sValue);
            }


            if (sName=="pixelsize") {
                edgeAnalysis2D.pixelSize= std::stod(sValue);
            }

            if (sName=="normalize") {
                edgeAnalysis2D.normalize=kipl::strings::string2bool(sValue);
            }

            if (sName=="useroi") {
                edgeAnalysis2D.useROI=kipl::strings::string2bool(sValue);
            }

            if (sName=="roi") {
                edgeAnalysis2D.roi.fromString(sValue);
            }

            if (sName=="makereport") {
                edgeAnalysis2D.makeReport=kipl::strings::string2bool(sValue);
            }
        }

        ret = xmlTextReaderRead(reader);
        if (xmlTextReaderDepth(reader)<depth)
            ret=0;
    }

}

void NIQAConfig::parseEdgeAnalysis3D(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    int ret = xmlTextReaderRead(reader);
    std::string sName, sValue;
    int depth=xmlTextReaderDepth(reader);

    while (ret == 1) {
        if (xmlTextReaderNodeType(reader)==1) {
            name = xmlTextReaderConstName(reader);
            ret=xmlTextReaderRead(reader);

            value = xmlTextReaderConstValue(reader);
            if (name==nullptr) {
                logger.error("Unexpected contents in parameter file");
                throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
            }
            if (value!=nullptr)
                sValue=reinterpret_cast<const char *>(value);
            else
                sValue="Empty";
            sName=reinterpret_cast<const char *>(name);

            if (sName=="filemask") {
                edgeAnalysis3D.fileMask=sValue;
            }

            if (sName=="first") {
                edgeAnalysis3D.first=std::stoi(sValue);
            }

            if (sName=="last") {
                edgeAnalysis3D.last=std::stoi(sValue);
            }

            if (sName=="step") {
                edgeAnalysis3D.step=std::stoi(sValue);
            }

            if (sName=="profilewidth") {
                edgeAnalysis3D.profileWidth = std::stod(sValue);
            }

            if (sName=="pixelsize") {
                edgeAnalysis3D.pixelSize= std::stod(sValue);
            }

            if (sName=="precision") {
                edgeAnalysis3D.precision = std::stod(sValue);
            }

            if (sName=="makereport") {
                edgeAnalysis3D.makeReport=kipl::strings::string2bool(sValue);
            }
        }

        ret = xmlTextReaderRead(reader);
        if (xmlTextReaderDepth(reader)<depth)
            ret=0;
    }
}

void NIQAConfig::parseBallPackingAnalysis(xmlTextReaderPtr reader)
{
    const xmlChar *name, *value;
    int ret = xmlTextReaderRead(reader);
    std::string sName, sValue;
    int depth=xmlTextReaderDepth(reader);
    std::ostringstream msg;
    while (ret == 1) {
        if (xmlTextReaderNodeType(reader)==1) {
            name = xmlTextReaderConstName(reader);
            ret=xmlTextReaderRead(reader);

            value = xmlTextReaderConstValue(reader);

            if (name==nullptr) {
                logger.error("Unexpected contents in parameter file");
                throw kipl::base::KiplException("Unexpected contents in parameter file",__FILE__,__LINE__);
            }
            sName=reinterpret_cast<const char *>(name);

            if (value!=nullptr) {
                sValue=reinterpret_cast<const char *>(value);
//                msg.str("");
//                msg<<"Got value "<<sValue<<" for field "<<sName;
//                logger.message(msg.str());
            }
            else {
                sValue="Empty";
                msg.str("");
                msg<<"Empty value for field "<<sName;
                logger.warning(msg.str());
            }

            if (sName=="filemask") {
                ballPackingAnalysis.fileMask=sValue;
            }

            if (sName=="first") {
                ballPackingAnalysis.first=std::stoi(sValue);
            }

            if (sName=="last") {
                ballPackingAnalysis.last=std::stoi(sValue);
            }

            if (sName=="step") {
                ballPackingAnalysis.step=std::stoi(sValue);
            }

            if (sName=="usecrop") {
                ballPackingAnalysis.useCrop = kipl::strings::string2bool(sValue);
            }

            if (sName=="crop") {
                ballPackingAnalysis.roi.fromString(sValue);
            }

            if (sName=="roilist") {
                std::vector<std::string> strlist;
                kipl::strings::String2Array(sValue,strlist);
                std::ostringstream msg;
                ballPackingAnalysis.analysisROIs.clear();

                msg<<"strlist.size: "<<strlist.size()<<", sValue: "<<sValue;
                logger.message(msg.str());
                if ((strlist.size() % 5 != 0) || (strlist.empty()==true)){
                    logger.error("Incomplete roi list");
                    throw kipl::base::KiplException("Incomplete ROI list",__FILE__,__LINE__);
                }

                for (size_t i=0; i<strlist.size(); i+=5)
                {
                    std::string lbl = strlist[i];
                    qDebug() << lbl.c_str();

                    std::vector<size_t> temproi = { std::stoul(strlist[i+1]),
                                                    std::stoul(strlist[i+2]),
                                                    std::stoul(strlist[i+3]),
                                                    std::stoul(strlist[i+4])};

                    kipl::base::RectROI roi(temproi,lbl);
                    ballPackingAnalysis.analysisROIs.push_back(roi);
                }

            }
            if (sName=="makereport") {
                ballPackingAnalysis.makeReport = kipl::strings::string2bool(sValue);
            }

        }
        ret = xmlTextReaderRead(reader);
        if (xmlTextReaderDepth(reader)<depth)
            ret=0;
    }
}

NIQAConfig::UserInformation::UserInformation() :
        logger("UserInformation"),
        userName("John Doe"),
        institute("Institute"),
        instrument("beamline"),
        country("Universe"),
        softwareVersion(VERSION),
        reportName("report.pdf")
{
    experimentDate[0]=2018;
    experimentDate[1]=7;
    experimentDate[2]=2;

    analysisDate[0]=2018;
    analysisDate[1]=7;
    analysisDate[2]=2;

}

NIQAConfig::UserInformation::UserInformation(const NIQAConfig::UserInformation &c) :
    logger("UserInformation"),
    userName(c.userName),
    institute(c.institute),
    instrument(c.instrument),
    country(c.country),
    softwareVersion(c.softwareVersion),
    comment(c.comment),
    reportName(c.reportName)
{
    std::copy_n(c.experimentDate,3,experimentDate);
    std::copy_n(c.analysisDate,3,analysisDate);
}

const NIQAConfig::UserInformation & NIQAConfig::UserInformation::operator=(const NIQAConfig::UserInformation &c)
{
    userName        = c.userName;
    institute       = c.institute;
    instrument      = c.instrument;
    country         = c.country;
    std::copy_n(c.experimentDate,3,experimentDate);
    std::copy_n(c.analysisDate,3,analysisDate);
    softwareVersion = c.softwareVersion;
    comment         = c.comment;
    reportName      = c.reportName;

    return *this;
}

std::string NIQAConfig::UserInformation::WriteXML(size_t indent)
{
    using namespace std;
    ostringstream str;

    str<<setw(indent)  <<" "<<"<userinformation>"<<std::endl;
        str<<setw(indent+4)  <<" "<<"<operator>"<<userName<<"</operator>\n";
        str<<setw(indent+4)  <<" "<<"<institute>"<<institute<<"</institute>\n";
        str<<setw(indent+4)  <<" "<<"<instrument>"<<instrument<<"</instrument>\n";
        str<<setw(indent+4)  <<" "<<"<country>"<<country<<"</country>\n";
        str<<setw(indent+4)  <<" "<<"<experimentdate>"<<experimentDate[0]<<" "<<experimentDate[1]<<" "<<experimentDate[2]<<" "<<"</experimentdate>\n";
        str<<setw(indent+4)  <<" "<<"<analysisdate>"<<analysisDate[0]<<" "<<analysisDate[1]<<" "<<analysisDate[2]<<" "<<"</analysisdate>\n";
        str<<setw(indent+4)  <<" "<<"<version>"<<softwareVersion<<"</version>\n";
        str<<setw(indent+4)  <<" "<<"<comment>"<<comment<<"</comment>\n";
        str<<setw(indent+4)  <<" "<<"<reportname>"<<reportName<<"</reportname>\n";

    str<<setw(indent)  <<" "<<"</userinformation>"<<std::endl;

    return str.str();
}


NIQAConfig::ContrastAnalysis::ContrastAnalysis() :
    logger("ContrastAnalysisConfig"),
    fileMask("img_####.fits"),
    first(0),
    last(1),
    step(1),
    pixelSize(0.1),
    intensitySlope(1.0),
    intensityIntercept(0.0),
    makeReport(false)
{

}

NIQAConfig::ContrastAnalysis::ContrastAnalysis(const ContrastAnalysis & c) :
    logger("ContrastAnalysisConfig"),
    fileMask(c.fileMask),
    first(c.first),
    last(c.last),
    step(c.step),
    pixelSize(c.pixelSize),
    intensitySlope(c.intensitySlope),
    intensityIntercept(c.intensityIntercept),
    analysisROIs(c.analysisROIs),
    makeReport(c.makeReport)
{

}

const NIQAConfig::ContrastAnalysis & NIQAConfig::ContrastAnalysis::operator=(const NIQAConfig::ContrastAnalysis &c)
{
        fileMask           = c.fileMask;
        first              = c.first;
        last               = c.last;
        step               = c.step;
        pixelSize          = c.pixelSize;
        intensitySlope     = c.intensitySlope;
        intensityIntercept = c.intensityIntercept;
        makeReport         = c.makeReport;
        analysisROIs       = c.analysisROIs;

        return *this;
}

std::string NIQAConfig::ContrastAnalysis::WriteXML(size_t indent)
{
    std::ostringstream str;

    str<<std::setw(indent)  <<" "<<"<contrastanalysis>"<<std::endl;
        str<<std::setw(indent+4)  <<" "<<"<filemask>"<<fileMask<<"</filemask>\n";
        str<<std::setw(indent+4)  <<" "<<"<first>"<<first<<"</first>\n";
        str<<std::setw(indent+4)  <<" "<<"<last>"<<last<<"</last>\n";
        str<<std::setw(indent+4)  <<" "<<"<step>"<<step<<"</step>\n";
        str<<std::setw(indent+4)  <<" "<<"<pixelsize>"<<kipl::strings::value2string(pixelSize)<<"</pixelsize>\n";
        str<<std::setw(indent+4)  <<" "<<"<intensityslope>"<<kipl::strings::value2string(intensitySlope)<<"</intensityslope>\n";
        str<<std::setw(indent+4)  <<" "<<"<intensityintercept>"<<kipl::strings::value2string(intensityIntercept)<<"</intensityintercept>\n";
        str<<std::setw(indent+4)  <<" "<<"<intensitymin>"<<kipl::strings::value2string(intensityMin)<<"</intensitymin>\n";
        str<<std::setw(indent+4)  <<" "<<"<intensitymax>"<<kipl::strings::value2string(intensityMax)<<"</intensitymax>\n";
        str<<std::setw(indent+4)  <<" "<<"<roilist>";
        for (auto it=analysisROIs.begin(); it!=analysisROIs.end(); ++it)
            str<<(it==analysisROIs.begin() ? "":" ")<<it->toString();
        str<<"</roilist>\n";
        str<<std::setw(indent+4)  <<" "<<"<makereport>"<<kipl::strings::bool2string(makeReport)<<"</makereport>\n";
    str<<std::setw(indent)  <<" "<<"</contrastanalysis>"<<std::endl;

    return str.str();
}

NIQAConfig::EdgeAnalysis2D::EdgeAnalysis2D() :
    logger("EdgeAnalysisConfig"),
    normalize(false),
    obMask("ob_#####.fits"),
    obFirst(0),
    obLast(4),
    obStep(1),
    dcMask("dc_#####.fits"),
    dcFirst(0),
    dcLast(1),
    dcStep(1),
    fitFunction(0),
    useROI(false),
    roi(0,0,1,1),
    pixelSize(0.1),
    makeReport(false)
{

}

NIQAConfig::EdgeAnalysis2D::EdgeAnalysis2D(const NIQAConfig::EdgeAnalysis2D &e) :
                   logger("EdgeAnalysisConfig"),
                   multiImageList(e.multiImageList),
                   normalize(e.normalize),
                   obMask(e.obMask),
                   obFirst(e.obFirst),
                   obLast(e.obLast),
                   obStep(e.obStep),
                   dcMask(e.dcMask),
                   dcFirst(e.dcFirst),
                   dcLast(e.dcLast),
                   dcStep(e.dcStep),
                   fitFunction(e.fitFunction),
                   useROI(e.useROI),
                   roi(e.roi),
                   pixelSize(e.pixelSize),
                   makeReport(e.makeReport)
{
}

const NIQAConfig::EdgeAnalysis2D & NIQAConfig::EdgeAnalysis2D::operator=(const NIQAConfig::EdgeAnalysis2D &e)
{
    multiImageList = e.multiImageList;
    normalize = e.normalize;
    obMask    = e.obMask;
    obFirst   = e.obFirst;
    obLast    = e.obLast;
    obStep    = e.obStep;
    dcMask    = e.dcMask;
    dcFirst   = e.dcFirst;
    dcLast    = e.dcLast;
    dcStep    = e.dcStep;
    fitFunction = e.fitFunction;
    useROI    = e.useROI;
    roi       = e.roi;
    pixelSize = e.pixelSize;
    makeReport= e.makeReport;

    return *this;
}

std::string NIQAConfig::EdgeAnalysis2D::WriteXML(size_t indent)
{
    std::ostringstream str;

    str<<std::setw(indent)  <<" "<<"<edgeanalysis2d>"<<std::endl;

        if (multiImageList.empty()==false) {
            str<<std::setw(indent+4)  <<" "<<"<multiimagelist>";
            for (auto it=multiImageList.begin(); it!=multiImageList.end(); ++it)
                str<<" "<<it->first<<" "<<it->second;

            str<<"</multiimagelist>\n";
        }

        str<<std::setw(indent+4)  <<" "<<"<normalize>"<<kipl::strings::bool2string(normalize)<<"</normalize>\n";
        str<<std::setw(indent+4)  <<" "<<"<obmask>"<<obMask<<"</obmask>\n";
        str<<std::setw(indent+4)  <<" "<<"<obfirst>"<<obFirst<<"</obfirst>\n";
        str<<std::setw(indent+4)  <<" "<<"<oblast>"<<obLast<<"</oblast>\n";
        str<<std::setw(indent+4)  <<" "<<"<obstep>"<<obStep<<"</obstep>\n";
        str<<std::setw(indent+4)  <<" "<<"<dcmask>"<<dcMask<<"</dcmask>\n";
        str<<std::setw(indent+4)  <<" "<<"<dcfirst>"<<dcFirst<<"</dcfirst>\n";
        str<<std::setw(indent+4)  <<" "<<"<dclast>"<<dcLast<<"</dclast>\n";
        str<<std::setw(indent+4)  <<" "<<"<dcstep>"<<dcStep<<"</dcstep>\n";
        str<<std::setw(indent+4)  <<" "<<"<fitfunction>"<<fitFunction<<"</fitfunction>\n";
        str<<std::setw(indent+4)  <<" "<<"<useroi>"<<kipl::strings::bool2string(useROI)<<"</useroi>\n";
        str<<std::setw(indent+4)  <<" "<<"<roi>"<<roi.toString()<<"</roi>\n";
        str<<std::setw(indent+4)  <<" "<<"<pixelsize>"<<kipl::strings::value2string(pixelSize)<<"</pixelsize>\n";
        str<<std::setw(indent+4)  <<" "<<"<makereport>"<<kipl::strings::bool2string(makeReport)<<"</makereport>\n";
    str<<std::setw(indent)  <<" "<<"</edgeanalysis2d>"<<std::endl;

    return str.str();
}

NIQAConfig::EdgeAnalysis3D::EdgeAnalysis3D() :
    logger("EdgeAnalysis3DConfig"),
    fileMask("slice_#####.tif"),
    first(0),
    last(1),
    step(1),
    pixelSize(0.1),
    profileWidth(40.0),
    precision(0.1),
    makeReport(false)
{

}

NIQAConfig::EdgeAnalysis3D::EdgeAnalysis3D(const NIQAConfig::EdgeAnalysis3D &e):
    logger("EdgeAnalysis3DConfig"),
    fileMask(e.fileMask),
    first(e.first),
    last(e.last),
    step(e.step),
    pixelSize(e.pixelSize),
    profileWidth(e.profileWidth),
    precision(e.precision),
    makeReport(e.makeReport)
{

}


const NIQAConfig::EdgeAnalysis3D & NIQAConfig::EdgeAnalysis3D::operator=(const NIQAConfig::EdgeAnalysis3D &e)
{
    fileMask=e.fileMask;
    first=e.first;
    last=e.last;
    step=e.step;
    pixelSize=e.pixelSize;
    profileWidth=e.profileWidth;
    precision=e.precision;
    makeReport=e.makeReport;

    return *this;
}

std::string NIQAConfig::EdgeAnalysis3D::WriteXML(size_t indent)
{
    std::ostringstream str;

    str<<std::setw(indent)  <<" "<<"<edgeanalysis3d>"<<std::endl;
        str<<std::setw(indent+4)  <<" "<<"<filemask>"<<fileMask<<"</filemask>\n";
        str<<std::setw(indent+4)  <<" "<<"<first>"<<first<<"</first>\n";
        str<<std::setw(indent+4)  <<" "<<"<last>"<<last<<"</last>\n";
        str<<std::setw(indent+4)  <<" "<<"<step>"<<step<<"</step>\n";
        str<<std::setw(indent+4)  <<" "<<"<pixelsize>"<<kipl::strings::value2string(pixelSize)<<"</pixelsize>\n";
        str<<std::setw(indent+4)  <<" "<<"<profilewidth>"<<kipl::strings::value2string(profileWidth)<<"</profilewidth>\n";
        str<<std::setw(indent+4)  <<" "<<"<precision>"<<kipl::strings::value2string(precision)<<"</precision>\n";
        str<<std::setw(indent+4)  <<" "<<"<makereport>"<<kipl::strings::bool2string(makeReport)<<"</makereport>\n";
    str<<std::setw(indent)  <<" "<<"</edgeanalysis3d>"<<std::endl;

    return str.str();
}


NIQAConfig::BallPackingAnalysis::BallPackingAnalysis() :
    logger("BallPackingAnalysisConfig"),
    fileMask("slice_#####.tif"),
    first(0),
    last(1),
    step(1),
    useCrop(false),
    roi(0,0,1,1),
    makeReport(false)
{}

NIQAConfig::BallPackingAnalysis::BallPackingAnalysis(const NIQAConfig::BallPackingAnalysis &c) :
    logger("BallPackingAnalysisConfig"),
    fileMask(c.fileMask),
    first(c.first),
    last(c.last),
    step(c.step),
    useCrop(c.useCrop),
    roi(c.roi),
    makeReport(c.makeReport)
{
    std::copy(c.analysisROIs.begin(),c.analysisROIs.end(),analysisROIs.begin());
}

const NIQAConfig::BallPackingAnalysis & NIQAConfig::BallPackingAnalysis::operator =(const NIQAConfig::BallPackingAnalysis & c)
{
    fileMask=c.fileMask;
    first=c.first;
    last=c.last;
    step=c.step;
    useCrop=c.useCrop;
    roi=c.roi;
    makeReport=c.makeReport;

    std::copy(c.analysisROIs.begin(),c.analysisROIs.end(),analysisROIs.begin());
    return *this;
}

std::string NIQAConfig::BallPackingAnalysis::WriteXML(size_t indent)
{
    std::ostringstream str;

    str<<std::setw(indent)  <<" "<<"<ballpacking>"<<std::endl;
        str<<std::setw(indent+4)  <<" "<<"<filemask>"<<fileMask<<"</filemask>\n";
        str<<std::setw(indent+4)  <<" "<<"<first>"<<first<<"</first>\n";
        str<<std::setw(indent+4)  <<" "<<"<last>"<<last<<"</last>\n";
        str<<std::setw(indent+4)  <<" "<<"<step>"<<step<<"</step>\n";
        str<<std::setw(indent+4)  <<" "<<"<usecrop>"<<kipl::strings::bool2string(useCrop)<<"</usecrop>\n";
        str<<std::setw(indent+4)  <<" "<<"<crop>"<<roi.toString()<<"</crop>\n";
        str<<std::setw(indent+4)  <<" "<<"<roilist>";
        for (auto it=analysisROIs.begin(); it!=analysisROIs.end(); ++it)
            str<<(it==analysisROIs.begin() ? "":" ")<<it->toString();
        str<<"</roilist>\n";
        str<<std::setw(indent+4)  <<" "<<"<makereport>"<<kipl::strings::bool2string(makeReport)<<"</makereport>\n";
    str<<std::setw(indent)  <<" "<<"</ballpacking>"<<std::endl;

    return str.str();
}


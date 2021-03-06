/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#include "DomainDisplayControl_001.h"
#include "XmlNode.h"
#include <algorithm>

DomainDisplayControl_001::DomainDisplayControl_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainDisplayControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainDisplayControl_001::~DomainDisplayControl_001(void)
{
}

DisplayControlDynamicCaps DomainDisplayControl_001::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    if (m_displayControlDynamicCaps.isInvalid())
    {
        m_displayControlDynamicCaps.set(createDisplayControlDynamicCaps(domainIndex));
    }
    return m_displayControlDynamicCaps.get();
}

DisplayControlStatus DomainDisplayControl_001::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    Percentage brightnessPercentage = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_DISPLAY_BRIGHTNESS, domainIndex);

    m_currentDisplayControlIndex.set(getDisplayControlSet(participantIndex, domainIndex).getControlIndex(brightnessPercentage));

    return DisplayControlStatus(m_currentDisplayControlIndex.get());
}

DisplayControlSet DomainDisplayControl_001::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
    if (m_displayControlSet.isInvalid())
    {
        m_displayControlSet.set(createDisplayControlSet(domainIndex));
    }
    return m_displayControlSet.get();
}

void DomainDisplayControl_001::setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
    throwIfControlIndexIsOutOfRange(displayControlIndex, domainIndex);

    auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
    Percentage newBrightness = displaySet[displayControlIndex].getBrightness();

    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_DISPLAY_BRIGHTNESS,
        newBrightness,
        domainIndex);

    m_currentDisplayControlIndex.set(displayControlIndex);
}

void DomainDisplayControl_001::setDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, 
    DisplayControlDynamicCaps newCapabilities)
{
    auto displaySet = getDisplayControlSet(participantIndex, domainIndex);
    auto upperLimitIndex = newCapabilities.getCurrentUpperLimit();
    auto lowerLimitIndex = newCapabilities.getCurrentLowerLimit();

    auto size = displaySet.getCount();
    if (upperLimitIndex >= size)
    {
        throw dptf_exception("Upper Limit index is out of control set bounds.");
    }
    else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= size)
    {
        lowerLimitIndex = size - 1;
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, setting lower limit to lowest possible index."));
    }

    m_displayControlDynamicCaps.invalidate();

    Percentage newUpperBrightness = displaySet[upperLimitIndex].getBrightness();
    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_DISPLAY_CAPABILITY,
        newUpperBrightness,
        domainIndex);

    Percentage newLowerBrightness = displaySet[lowerLimitIndex].getBrightness();
    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_DISPLAY_DEPTH_LIMIT,
        newLowerBrightness,
        domainIndex);
}

void DomainDisplayControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    if (isActivityLoggingEnabled() == true) 
    {
        auto dynamicCaps = getDisplayControlDynamicCaps(participantIndex, domainIndex);
        auto displaySet = getDisplayControlSet(participantIndex, domainIndex);

        UInt32 displayControlIndex;
        if (m_currentDisplayControlIndex.isInvalid())
        {
            displayControlIndex = dynamicCaps.getCurrentUpperLimit();
        }
        else
        {
            displayControlIndex = m_currentDisplayControlIndex.get();
        }

        EsifCapabilityData capability;
        capability.type = Capability::DisplayControl;
        capability.size = sizeof(capability);
        capability.data.displayControl.currentDPTFLimit = displaySet[displayControlIndex].getBrightness().toWholeNumber();
        capability.data.displayControl.lowerLimit = displaySet[dynamicCaps.getCurrentLowerLimit()].getBrightness().toWholeNumber();
        capability.data.displayControl.upperLimit = displaySet[dynamicCaps.getCurrentUpperLimit()].getBrightness().toWholeNumber();

        getParticipantServices()->sendDptfEvent(ParticipantEvent::DptfParticipantControlAction,
            domainIndex, Capability::getEsifDataFromCapabilityData(&capability));
    }
}

void DomainDisplayControl_001::clearCachedData(void)
{
    m_displayControlDynamicCaps.invalidate();
    m_displayControlSet.invalidate();
    m_currentDisplayControlIndex.invalidate();
}

std::shared_ptr<XmlNode> DomainDisplayControl_001::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("display_control");
    root->addChild(getDisplayControlStatus(Constants::Invalid, domainIndex).getXml());
    root->addChild(getDisplayControlDynamicCaps(Constants::Invalid, domainIndex).getXml());
    root->addChild(getDisplayControlSet(Constants::Invalid, domainIndex).getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

DisplayControlDynamicCaps DomainDisplayControl_001::createDisplayControlDynamicCaps(UIntN domainIndex)
{
    auto displaySet = getDisplayControlSet(Constants::Invalid, domainIndex);

    // Get dynamic caps
    //  The caps are stored in BIOS as brightness percentage.  They must be converted
    //  to indices before they can be used.
    UIntN lowerLimitIndex = getLowerLimitIndex(domainIndex, displaySet);
    UIntN upperLimitIndex = getUpperLimitIndex(domainIndex, displaySet);

    auto size = displaySet.getCount();
    if (upperLimitIndex >= size)
    {
        throw dptf_exception("Upper Limit index is out of control set bounds.");
    }
    else if (upperLimitIndex > lowerLimitIndex || lowerLimitIndex >= size)
    {
        lowerLimitIndex = size - 1;
        getParticipantServices()->writeMessageWarning(
            ParticipantMessage(FLF, "Limit index mismatch, ignoring lower limit."));
    }

    return DisplayControlDynamicCaps(upperLimitIndex, lowerLimitIndex);
}

DisplayControlSet DomainDisplayControl_001::createDisplayControlSet(UIntN domainIndex)
{
    // _BCL Table
    DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_DISPLAY_BRIGHTNESS_LEVELS, ESIF_DATA_BINARY, domainIndex);
    std::vector<DisplayControl> controls = BinaryParse::displayBclObject(buffer);
    throwIfDisplaySetIsEmpty((UIntN)controls.size());

    return DisplayControlSet(processDisplayControlSetData(controls));
}

void DomainDisplayControl_001::throwIfControlIndexIsOutOfRange(UIntN displayControlIndex, UIntN domainIndex)
{
    auto dynamicCaps = getDisplayControlDynamicCaps(Constants::Invalid, domainIndex);
    auto upperLimit = dynamicCaps.getCurrentUpperLimit();
    auto lowerLimit = dynamicCaps.getCurrentLowerLimit();
    auto displaySet = getDisplayControlSet(Constants::Invalid, domainIndex);
    auto size = displaySet.getCount();

    if (displayControlIndex >= size || displayControlIndex < upperLimit || displayControlIndex > lowerLimit)
    {
        std::stringstream infoMessage;

        infoMessage << "Control index is outside the allowable range." << std::endl
            << "Desired Index : " << displayControlIndex << std::endl
            << "DisplayControlSet size :" << size << std::endl
            << "Upper Limit : " << upperLimit << std::endl
            << "Lower Limit : " << lowerLimit << std::endl;

        throw dptf_exception(infoMessage.str());
    }
}

void DomainDisplayControl_001::throwIfDisplaySetIsEmpty(UIntN sizeOfSet)
{
    if (sizeOfSet == 0)
    {
        throw dptf_exception("Display Brightness set is empty. Impossible if we support display controls.");
    }
}

UIntN DomainDisplayControl_001::getLowerLimitIndex(UIntN domainIndex, DisplayControlSet displaySet)
{
    UInt32 uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_DISPLAY_DEPTH_LIMIT, domainIndex);
    Percentage lowerLimitBrightness = Percentage::fromWholeNumber(uint32val);
    return displaySet.getControlIndex(lowerLimitBrightness);
}

UIntN DomainDisplayControl_001::getUpperLimitIndex(UIntN domainIndex, DisplayControlSet displaySet)
{
    UIntN upperLimitIndex;
    try
    {
        UInt32 uint32val = getParticipantServices()->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_DISPLAY_CAPABILITY, domainIndex);
        Percentage upperLimitBrightness = Percentage::fromWholeNumber(uint32val);
        upperLimitIndex = displaySet.getControlIndex(upperLimitBrightness);
    }
    catch (...)
    {
        // DDPC is optional
        getParticipantServices()->writeMessageDebug(
            ParticipantMessage(FLF, "DDPC was not present.  Setting upper limit to 100."));
        upperLimitIndex = 0; // Max brightness
    }
    return upperLimitIndex;
}

std::vector<DisplayControl> DomainDisplayControl_001::processDisplayControlSetData(std::vector<DisplayControl> controls)
{
    std::sort(controls.begin(), controls.end());
    controls.erase(std::unique(controls.begin(), controls.end()), controls.end());  // remove duplicates
    std::reverse(controls.begin(), controls.end());

    return controls;
}

std::string DomainDisplayControl_001::getName(void)
{
    return "Display Control (Version 1)";
}
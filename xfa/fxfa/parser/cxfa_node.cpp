// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_node.h"

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_accessiblecontent.h"
#include "xfa/fxfa/parser/cxfa_acrobat.h"
#include "xfa/fxfa/parser/cxfa_acrobat7.h"
#include "xfa/fxfa/parser/cxfa_adbe_jsconsole.h"
#include "xfa/fxfa/parser/cxfa_adbe_jsdebugger.h"
#include "xfa/fxfa/parser/cxfa_addsilentprint.h"
#include "xfa/fxfa/parser/cxfa_addviewerpreferences.h"
#include "xfa/fxfa/parser/cxfa_adjustdata.h"
#include "xfa/fxfa/parser/cxfa_adobeextensionlevel.h"
#include "xfa/fxfa/parser/cxfa_agent.h"
#include "xfa/fxfa/parser/cxfa_alwaysembed.h"
#include "xfa/fxfa/parser/cxfa_amd.h"
#include "xfa/fxfa/parser/cxfa_appearancefilter.h"
#include "xfa/fxfa/parser/cxfa_arc.h"
#include "xfa/fxfa/parser/cxfa_area.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_assist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_attributes.h"
#include "xfa/fxfa/parser/cxfa_autosave.h"
#include "xfa/fxfa/parser/cxfa_barcode.h"
#include "xfa/fxfa/parser/cxfa_base.h"
#include "xfa/fxfa/parser/cxfa_batchoutput.h"
#include "xfa/fxfa/parser/cxfa_behavioroverride.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_binditems.h"
#include "xfa/fxfa/parser/cxfa_bookend.h"
#include "xfa/fxfa/parser/cxfa_boolean.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_break.h"
#include "xfa/fxfa/parser/cxfa_breakafter.h"
#include "xfa/fxfa/parser/cxfa_breakbefore.h"
#include "xfa/fxfa/parser/cxfa_button.h"
#include "xfa/fxfa/parser/cxfa_cache.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_certificate.h"
#include "xfa/fxfa/parser/cxfa_certificates.h"
#include "xfa/fxfa/parser/cxfa_change.h"
#include "xfa/fxfa/parser/cxfa_checkbutton.h"
#include "xfa/fxfa/parser/cxfa_choicelist.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_comb.h"
#include "xfa/fxfa/parser/cxfa_command.h"
#include "xfa/fxfa/parser/cxfa_common.h"
#include "xfa/fxfa/parser/cxfa_compress.h"
#include "xfa/fxfa/parser/cxfa_compression.h"
#include "xfa/fxfa/parser/cxfa_compresslogicalstructure.h"
#include "xfa/fxfa/parser/cxfa_compressobjectstream.h"
#include "xfa/fxfa/parser/cxfa_config.h"
#include "xfa/fxfa/parser/cxfa_conformance.h"
#include "xfa/fxfa/parser/cxfa_connect.h"
#include "xfa/fxfa/parser/cxfa_connectionset.h"
#include "xfa/fxfa/parser/cxfa_connectstring.h"
#include "xfa/fxfa/parser/cxfa_contentarea.h"
#include "xfa/fxfa/parser/cxfa_contentcopy.h"
#include "xfa/fxfa/parser/cxfa_copies.h"
#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_creator.h"
#include "xfa/fxfa/parser/cxfa_currencysymbol.h"
#include "xfa/fxfa/parser/cxfa_currencysymbols.h"
#include "xfa/fxfa/parser/cxfa_currentpage.h"
#include "xfa/fxfa/parser/cxfa_data.h"
#include "xfa/fxfa/parser/cxfa_datagroup.h"
#include "xfa/fxfa/parser/cxfa_datamodel.h"
#include "xfa/fxfa/parser/cxfa_datavalue.h"
#include "xfa/fxfa/parser/cxfa_datawindow.h"
#include "xfa/fxfa/parser/cxfa_date.h"
#include "xfa/fxfa/parser/cxfa_datepattern.h"
#include "xfa/fxfa/parser/cxfa_datepatterns.h"
#include "xfa/fxfa/parser/cxfa_datetime.h"
#include "xfa/fxfa/parser/cxfa_datetimeedit.h"
#include "xfa/fxfa/parser/cxfa_datetimesymbols.h"
#include "xfa/fxfa/parser/cxfa_day.h"
#include "xfa/fxfa/parser/cxfa_daynames.h"
#include "xfa/fxfa/parser/cxfa_debug.h"
#include "xfa/fxfa/parser/cxfa_decimal.h"
#include "xfa/fxfa/parser/cxfa_defaulttypeface.h"
#include "xfa/fxfa/parser/cxfa_defaultui.h"
#include "xfa/fxfa/parser/cxfa_delete.h"
#include "xfa/fxfa/parser/cxfa_delta.h"
#include "xfa/fxfa/parser/cxfa_deltas.h"
#include "xfa/fxfa/parser/cxfa_desc.h"
#include "xfa/fxfa/parser/cxfa_destination.h"
#include "xfa/fxfa/parser/cxfa_digestmethod.h"
#include "xfa/fxfa/parser/cxfa_digestmethods.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_documentassembly.h"
#include "xfa/fxfa/parser/cxfa_draw.h"
#include "xfa/fxfa/parser/cxfa_driver.h"
#include "xfa/fxfa/parser/cxfa_dsigdata.h"
#include "xfa/fxfa/parser/cxfa_duplexoption.h"
#include "xfa/fxfa/parser/cxfa_dynamicrender.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_effectiveinputpolicy.h"
#include "xfa/fxfa/parser/cxfa_effectiveoutputpolicy.h"
#include "xfa/fxfa/parser/cxfa_embed.h"
#include "xfa/fxfa/parser/cxfa_encoding.h"
#include "xfa/fxfa/parser/cxfa_encodings.h"
#include "xfa/fxfa/parser/cxfa_encrypt.h"
#include "xfa/fxfa/parser/cxfa_encryption.h"
#include "xfa/fxfa/parser/cxfa_encryptionlevel.h"
#include "xfa/fxfa/parser/cxfa_encryptionmethod.h"
#include "xfa/fxfa/parser/cxfa_encryptionmethods.h"
#include "xfa/fxfa/parser/cxfa_enforce.h"
#include "xfa/fxfa/parser/cxfa_equate.h"
#include "xfa/fxfa/parser/cxfa_equaterange.h"
#include "xfa/fxfa/parser/cxfa_era.h"
#include "xfa/fxfa/parser/cxfa_eranames.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_eventpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_exclgroup.h"
#include "xfa/fxfa/parser/cxfa_exclude.h"
#include "xfa/fxfa/parser/cxfa_excludens.h"
#include "xfa/fxfa/parser/cxfa_exdata.h"
#include "xfa/fxfa/parser/cxfa_execute.h"
#include "xfa/fxfa/parser/cxfa_exobject.h"
#include "xfa/fxfa/parser/cxfa_extras.h"
#include "xfa/fxfa/parser/cxfa_field.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_filter.h"
#include "xfa/fxfa/parser/cxfa_fliplabel.h"
#include "xfa/fxfa/parser/cxfa_float.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_fontinfo.h"
#include "xfa/fxfa/parser/cxfa_form.h"
#include "xfa/fxfa/parser/cxfa_format.h"
#include "xfa/fxfa/parser/cxfa_formfieldfilling.h"
#include "xfa/fxfa/parser/cxfa_groupparent.h"
#include "xfa/fxfa/parser/cxfa_handler.h"
#include "xfa/fxfa/parser/cxfa_hostpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_hyphenation.h"
#include "xfa/fxfa/parser/cxfa_ifempty.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_imageedit.h"
#include "xfa/fxfa/parser/cxfa_includexdpcontent.h"
#include "xfa/fxfa/parser/cxfa_incrementalload.h"
#include "xfa/fxfa/parser/cxfa_incrementalmerge.h"
#include "xfa/fxfa/parser/cxfa_insert.h"
#include "xfa/fxfa/parser/cxfa_instancemanager.h"
#include "xfa/fxfa/parser/cxfa_integer.h"
#include "xfa/fxfa/parser/cxfa_interactive.h"
#include "xfa/fxfa/parser/cxfa_issuers.h"
#include "xfa/fxfa/parser/cxfa_items.h"
#include "xfa/fxfa/parser/cxfa_jog.h"
#include "xfa/fxfa/parser/cxfa_keep.h"
#include "xfa/fxfa/parser/cxfa_keyusage.h"
#include "xfa/fxfa/parser/cxfa_labelprinter.h"
#include "xfa/fxfa/parser/cxfa_layout.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_layoutpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_level.h"
#include "xfa/fxfa/parser/cxfa_line.h"
#include "xfa/fxfa/parser/cxfa_linear.h"
#include "xfa/fxfa/parser/cxfa_linearized.h"
#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_locale.h"
#include "xfa/fxfa/parser/cxfa_localeset.h"
#include "xfa/fxfa/parser/cxfa_lockdocument.h"
#include "xfa/fxfa/parser/cxfa_log.h"
#include "xfa/fxfa/parser/cxfa_logpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_manifest.h"
#include "xfa/fxfa/parser/cxfa_map.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_mdp.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_medium.h"
#include "xfa/fxfa/parser/cxfa_mediuminfo.h"
#include "xfa/fxfa/parser/cxfa_meridiem.h"
#include "xfa/fxfa/parser/cxfa_meridiemnames.h"
#include "xfa/fxfa/parser/cxfa_message.h"
#include "xfa/fxfa/parser/cxfa_messaging.h"
#include "xfa/fxfa/parser/cxfa_mode.h"
#include "xfa/fxfa/parser/cxfa_modifyannots.h"
#include "xfa/fxfa/parser/cxfa_month.h"
#include "xfa/fxfa/parser/cxfa_monthnames.h"
#include "xfa/fxfa/parser/cxfa_msgid.h"
#include "xfa/fxfa/parser/cxfa_nameattr.h"
#include "xfa/fxfa/parser/cxfa_neverembed.h"
#include "xfa/fxfa/parser/cxfa_numberofcopies.h"
#include "xfa/fxfa/parser/cxfa_numberpattern.h"
#include "xfa/fxfa/parser/cxfa_numberpatterns.h"
#include "xfa/fxfa/parser/cxfa_numbersymbol.h"
#include "xfa/fxfa/parser/cxfa_numbersymbols.h"
#include "xfa/fxfa/parser/cxfa_numericedit.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"
#include "xfa/fxfa/parser/cxfa_oid.h"
#include "xfa/fxfa/parser/cxfa_oids.h"
#include "xfa/fxfa/parser/cxfa_openaction.h"
#include "xfa/fxfa/parser/cxfa_operation.h"
#include "xfa/fxfa/parser/cxfa_output.h"
#include "xfa/fxfa/parser/cxfa_outputbin.h"
#include "xfa/fxfa/parser/cxfa_outputxsl.h"
#include "xfa/fxfa/parser/cxfa_overflow.h"
#include "xfa/fxfa/parser/cxfa_overprint.h"
#include "xfa/fxfa/parser/cxfa_packet.h"
#include "xfa/fxfa/parser/cxfa_packets.h"
#include "xfa/fxfa/parser/cxfa_pagearea.h"
#include "xfa/fxfa/parser/cxfa_pageoffset.h"
#include "xfa/fxfa/parser/cxfa_pagerange.h"
#include "xfa/fxfa/parser/cxfa_pageset.h"
#include "xfa/fxfa/parser/cxfa_pagination.h"
#include "xfa/fxfa/parser/cxfa_paginationoverride.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_part.h"
#include "xfa/fxfa/parser/cxfa_password.h"
#include "xfa/fxfa/parser/cxfa_passwordedit.h"
#include "xfa/fxfa/parser/cxfa_pattern.h"
#include "xfa/fxfa/parser/cxfa_pcl.h"
#include "xfa/fxfa/parser/cxfa_pdf.h"
#include "xfa/fxfa/parser/cxfa_pdfa.h"
#include "xfa/fxfa/parser/cxfa_permissions.h"
#include "xfa/fxfa/parser/cxfa_picktraybypdfsize.h"
#include "xfa/fxfa/parser/cxfa_picture.h"
#include "xfa/fxfa/parser/cxfa_plaintextmetadata.h"
#include "xfa/fxfa/parser/cxfa_presence.h"
#include "xfa/fxfa/parser/cxfa_present.h"
#include "xfa/fxfa/parser/cxfa_print.h"
#include "xfa/fxfa/parser/cxfa_printername.h"
#include "xfa/fxfa/parser/cxfa_printhighquality.h"
#include "xfa/fxfa/parser/cxfa_printscaling.h"
#include "xfa/fxfa/parser/cxfa_producer.h"
#include "xfa/fxfa/parser/cxfa_proto.h"
#include "xfa/fxfa/parser/cxfa_ps.h"
#include "xfa/fxfa/parser/cxfa_psmap.h"
#include "xfa/fxfa/parser/cxfa_query.h"
#include "xfa/fxfa/parser/cxfa_radial.h"
#include "xfa/fxfa/parser/cxfa_range.h"
#include "xfa/fxfa/parser/cxfa_reason.h"
#include "xfa/fxfa/parser/cxfa_reasons.h"
#include "xfa/fxfa/parser/cxfa_record.h"
#include "xfa/fxfa/parser/cxfa_recordset.h"
#include "xfa/fxfa/parser/cxfa_rectangle.h"
#include "xfa/fxfa/parser/cxfa_ref.h"
#include "xfa/fxfa/parser/cxfa_relevant.h"
#include "xfa/fxfa/parser/cxfa_rename.h"
#include "xfa/fxfa/parser/cxfa_renderpolicy.h"
#include "xfa/fxfa/parser/cxfa_rootelement.h"
#include "xfa/fxfa/parser/cxfa_runscripts.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_scriptmodel.h"
#include "xfa/fxfa/parser/cxfa_select.h"
#include "xfa/fxfa/parser/cxfa_setproperty.h"
#include "xfa/fxfa/parser/cxfa_severity.h"
#include "xfa/fxfa/parser/cxfa_signature.h"
#include "xfa/fxfa/parser/cxfa_signatureproperties.h"
#include "xfa/fxfa/parser/cxfa_signaturepseudomodel.h"
#include "xfa/fxfa/parser/cxfa_signdata.h"
#include "xfa/fxfa/parser/cxfa_signing.h"
#include "xfa/fxfa/parser/cxfa_silentprint.h"
#include "xfa/fxfa/parser/cxfa_simple_parser.h"
#include "xfa/fxfa/parser/cxfa_soapaction.h"
#include "xfa/fxfa/parser/cxfa_soapaddress.h"
#include "xfa/fxfa/parser/cxfa_solid.h"
#include "xfa/fxfa/parser/cxfa_source.h"
#include "xfa/fxfa/parser/cxfa_sourceset.h"
#include "xfa/fxfa/parser/cxfa_speak.h"
#include "xfa/fxfa/parser/cxfa_staple.h"
#include "xfa/fxfa/parser/cxfa_startnode.h"
#include "xfa/fxfa/parser/cxfa_startpage.h"
#include "xfa/fxfa/parser/cxfa_stipple.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_subformset.h"
#include "xfa/fxfa/parser/cxfa_subjectdn.h"
#include "xfa/fxfa/parser/cxfa_subjectdns.h"
#include "xfa/fxfa/parser/cxfa_submit.h"
#include "xfa/fxfa/parser/cxfa_submitformat.h"
#include "xfa/fxfa/parser/cxfa_submiturl.h"
#include "xfa/fxfa/parser/cxfa_subsetbelow.h"
#include "xfa/fxfa/parser/cxfa_suppressbanner.h"
#include "xfa/fxfa/parser/cxfa_tagged.h"
#include "xfa/fxfa/parser/cxfa_template.h"
#include "xfa/fxfa/parser/cxfa_templatecache.h"
#include "xfa/fxfa/parser/cxfa_text.h"
#include "xfa/fxfa/parser/cxfa_textedit.h"
#include "xfa/fxfa/parser/cxfa_threshold.h"
#include "xfa/fxfa/parser/cxfa_time.h"
#include "xfa/fxfa/parser/cxfa_timepattern.h"
#include "xfa/fxfa/parser/cxfa_timepatterns.h"
#include "xfa/fxfa/parser/cxfa_timestamp.h"
#include "xfa/fxfa/parser/cxfa_to.h"
#include "xfa/fxfa/parser/cxfa_tooltip.h"
#include "xfa/fxfa/parser/cxfa_trace.h"
#include "xfa/fxfa/parser/cxfa_transform.h"
#include "xfa/fxfa/parser/cxfa_traversal.h"
#include "xfa/fxfa/parser/cxfa_traverse.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_type.h"
#include "xfa/fxfa/parser/cxfa_typeface.h"
#include "xfa/fxfa/parser/cxfa_typefaces.h"
#include "xfa/fxfa/parser/cxfa_ui.h"
#include "xfa/fxfa/parser/cxfa_update.h"
#include "xfa/fxfa/parser/cxfa_uri.h"
#include "xfa/fxfa/parser/cxfa_user.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/cxfa_validateapprovalsignatures.h"
#include "xfa/fxfa/parser/cxfa_validationmessaging.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/cxfa_variables.h"
#include "xfa/fxfa/parser/cxfa_version.h"
#include "xfa/fxfa/parser/cxfa_versioncontrol.h"
#include "xfa/fxfa/parser/cxfa_viewerpreferences.h"
#include "xfa/fxfa/parser/cxfa_webclient.h"
#include "xfa/fxfa/parser/cxfa_whitespace.h"
#include "xfa/fxfa/parser/cxfa_window.h"
#include "xfa/fxfa/parser/cxfa_wsdladdress.h"
#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"
#include "xfa/fxfa/parser/cxfa_xdc.h"
#include "xfa/fxfa/parser/cxfa_xdp.h"
#include "xfa/fxfa/parser/cxfa_xfa.h"
#include "xfa/fxfa/parser/cxfa_xmlconnection.h"
#include "xfa/fxfa/parser/cxfa_xsdconnection.h"
#include "xfa/fxfa/parser/cxfa_xsl.h"
#include "xfa/fxfa/parser/cxfa_zpl.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"

namespace {

void XFA_DataNodeDeleteBindItem(void* pData) {
  delete static_cast<std::vector<CXFA_Node*>*>(pData);
}

XFA_MAPDATABLOCKCALLBACKINFO deleteBindItemCallBack = {
    XFA_DataNodeDeleteBindItem, nullptr};

std::vector<CXFA_Node*> NodesSortedByDocumentIdx(
    const std::set<CXFA_Node*>& rgNodeSet) {
  if (rgNodeSet.empty())
    return std::vector<CXFA_Node*>();

  std::vector<CXFA_Node*> rgNodeArray;
  CXFA_Node* pCommonParent =
      (*rgNodeSet.begin())->GetNodeItem(XFA_NODEITEM_Parent);
  for (CXFA_Node* pNode = pCommonParent->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pdfium::ContainsValue(rgNodeSet, pNode))
      rgNodeArray.push_back(pNode);
  }
  return rgNodeArray;
}

using CXFA_NodeSetPair = std::pair<std::set<CXFA_Node*>, std::set<CXFA_Node*>>;
using CXFA_NodeSetPairMap =
    std::map<uint32_t, std::unique_ptr<CXFA_NodeSetPair>>;
using CXFA_NodeSetPairMapMap =
    std::map<CXFA_Node*, std::unique_ptr<CXFA_NodeSetPairMap>>;

CXFA_NodeSetPair* NodeSetPairForNode(CXFA_Node* pNode,
                                     CXFA_NodeSetPairMapMap* pMap) {
  CXFA_Node* pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  uint32_t dwNameHash = pNode->GetNameHash();
  if (!pParentNode || !dwNameHash)
    return nullptr;

  if (!(*pMap)[pParentNode])
    (*pMap)[pParentNode] = pdfium::MakeUnique<CXFA_NodeSetPairMap>();

  CXFA_NodeSetPairMap* pNodeSetPairMap = (*pMap)[pParentNode].get();
  if (!(*pNodeSetPairMap)[dwNameHash])
    (*pNodeSetPairMap)[dwNameHash] = pdfium::MakeUnique<CXFA_NodeSetPair>();

  return (*pNodeSetPairMap)[dwNameHash].get();
}

void ReorderDataNodes(const std::set<CXFA_Node*>& sSet1,
                      const std::set<CXFA_Node*>& sSet2,
                      bool bInsertBefore) {
  CXFA_NodeSetPairMapMap rgMap;
  for (CXFA_Node* pNode : sSet1) {
    CXFA_NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair)
      pNodeSetPair->first.insert(pNode);
  }
  for (CXFA_Node* pNode : sSet2) {
    CXFA_NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair) {
      if (pdfium::ContainsValue(pNodeSetPair->first, pNode))
        pNodeSetPair->first.erase(pNode);
      else
        pNodeSetPair->second.insert(pNode);
    }
  }
  for (const auto& iter1 : rgMap) {
    CXFA_NodeSetPairMap* pNodeSetPairMap = iter1.second.get();
    if (!pNodeSetPairMap)
      continue;

    for (const auto& iter2 : *pNodeSetPairMap) {
      CXFA_NodeSetPair* pNodeSetPair = iter2.second.get();
      if (!pNodeSetPair)
        continue;
      if (!pNodeSetPair->first.empty() && !pNodeSetPair->second.empty()) {
        std::vector<CXFA_Node*> rgNodeArray1 =
            NodesSortedByDocumentIdx(pNodeSetPair->first);
        std::vector<CXFA_Node*> rgNodeArray2 =
            NodesSortedByDocumentIdx(pNodeSetPair->second);
        CXFA_Node* pParentNode = nullptr;
        CXFA_Node* pBeforeNode = nullptr;
        if (bInsertBefore) {
          pBeforeNode = rgNodeArray2.front();
          pParentNode = pBeforeNode->GetNodeItem(XFA_NODEITEM_Parent);
        } else {
          CXFA_Node* pLastNode = rgNodeArray2.back();
          pParentNode = pLastNode->GetNodeItem(XFA_NODEITEM_Parent);
          pBeforeNode = pLastNode->GetNodeItem(XFA_NODEITEM_NextSibling);
        }
        for (auto* pCurNode : rgNodeArray1) {
          pParentNode->RemoveChild(pCurNode, true);
          pParentNode->InsertChild(pCurNode, pBeforeNode);
        }
      }
    }
    pNodeSetPairMap->clear();
  }
}

}  // namespace

const XFA_ATTRIBUTEENUMINFO* GetAttributeEnumByID(XFA_ATTRIBUTEENUM eName) {
  return g_XFAEnumData + eName;
}

// static
std::unique_ptr<CXFA_Node> CXFA_Node::Create(CXFA_Document* doc,
                                             XFA_XDPPACKET packet,
                                             const XFA_ELEMENTINFO* pElement) {
  ASSERT(pElement->eName != XFA_Element::NodeList);
  switch (pElement->eName) {
    case XFA_Element::Ps:
      return pdfium::MakeUnique<CXFA_Ps>(doc, packet);
    case XFA_Element::To:
      return pdfium::MakeUnique<CXFA_To>(doc, packet);
    case XFA_Element::Ui:
      return pdfium::MakeUnique<CXFA_Ui>(doc, packet);
    case XFA_Element::RecordSet:
      return pdfium::MakeUnique<CXFA_RecordSet>(doc, packet);
    case XFA_Element::SubsetBelow:
      return pdfium::MakeUnique<CXFA_SubsetBelow>(doc, packet);
    case XFA_Element::SubformSet:
      return pdfium::MakeUnique<CXFA_SubformSet>(doc, packet);
    case XFA_Element::AdobeExtensionLevel:
      return pdfium::MakeUnique<CXFA_AdobeExtensionLevel>(doc, packet);
    case XFA_Element::Typeface:
      return pdfium::MakeUnique<CXFA_Typeface>(doc, packet);
    case XFA_Element::Break:
      return pdfium::MakeUnique<CXFA_Break>(doc, packet);
    case XFA_Element::FontInfo:
      return pdfium::MakeUnique<CXFA_FontInfo>(doc, packet);
    case XFA_Element::NumberPattern:
      return pdfium::MakeUnique<CXFA_NumberPattern>(doc, packet);
    case XFA_Element::DynamicRender:
      return pdfium::MakeUnique<CXFA_DynamicRender>(doc, packet);
    case XFA_Element::PrintScaling:
      return pdfium::MakeUnique<CXFA_PrintScaling>(doc, packet);
    case XFA_Element::CheckButton:
      return pdfium::MakeUnique<CXFA_CheckButton>(doc, packet);
    case XFA_Element::DatePatterns:
      return pdfium::MakeUnique<CXFA_DatePatterns>(doc, packet);
    case XFA_Element::SourceSet:
      return pdfium::MakeUnique<CXFA_SourceSet>(doc, packet);
    case XFA_Element::Amd:
      return pdfium::MakeUnique<CXFA_Amd>(doc, packet);
    case XFA_Element::Arc:
      return pdfium::MakeUnique<CXFA_Arc>(doc, packet);
    case XFA_Element::Day:
      return pdfium::MakeUnique<CXFA_Day>(doc, packet);
    case XFA_Element::Era:
      return pdfium::MakeUnique<CXFA_Era>(doc, packet);
    case XFA_Element::Jog:
      return pdfium::MakeUnique<CXFA_Jog>(doc, packet);
    case XFA_Element::Log:
      return pdfium::MakeUnique<CXFA_Log>(doc, packet);
    case XFA_Element::Map:
      return pdfium::MakeUnique<CXFA_Map>(doc, packet);
    case XFA_Element::Mdp:
      return pdfium::MakeUnique<CXFA_Mdp>(doc, packet);
    case XFA_Element::BreakBefore:
      return pdfium::MakeUnique<CXFA_BreakBefore>(doc, packet);
    case XFA_Element::Oid:
      return pdfium::MakeUnique<CXFA_Oid>(doc, packet);
    case XFA_Element::Pcl:
      return pdfium::MakeUnique<CXFA_Pcl>(doc, packet);
    case XFA_Element::Pdf:
      return pdfium::MakeUnique<CXFA_Pdf>(doc, packet);
    case XFA_Element::Ref:
      return pdfium::MakeUnique<CXFA_Ref>(doc, packet);
    case XFA_Element::Uri:
      return pdfium::MakeUnique<CXFA_Uri>(doc, packet);
    case XFA_Element::Xdc:
      return pdfium::MakeUnique<CXFA_Xdc>(doc, packet);
    case XFA_Element::Xdp:
      return pdfium::MakeUnique<CXFA_Xdp>(doc, packet);
    case XFA_Element::Xfa:
      return pdfium::MakeUnique<CXFA_Xfa>(doc, packet);
    case XFA_Element::Xsl:
      return pdfium::MakeUnique<CXFA_Xsl>(doc, packet);
    case XFA_Element::Zpl:
      return pdfium::MakeUnique<CXFA_Zpl>(doc, packet);
    case XFA_Element::Cache:
      return pdfium::MakeUnique<CXFA_Cache>(doc, packet);
    case XFA_Element::Margin:
      return pdfium::MakeUnique<CXFA_Margin>(doc, packet);
    case XFA_Element::KeyUsage:
      return pdfium::MakeUnique<CXFA_KeyUsage>(doc, packet);
    case XFA_Element::Exclude:
      return pdfium::MakeUnique<CXFA_Exclude>(doc, packet);
    case XFA_Element::ChoiceList:
      return pdfium::MakeUnique<CXFA_ChoiceList>(doc, packet);
    case XFA_Element::Level:
      return pdfium::MakeUnique<CXFA_Level>(doc, packet);
    case XFA_Element::LabelPrinter:
      return pdfium::MakeUnique<CXFA_LabelPrinter>(doc, packet);
    case XFA_Element::CalendarSymbols:
      return pdfium::MakeUnique<CXFA_CalendarSymbols>(doc, packet);
    case XFA_Element::Para:
      return pdfium::MakeUnique<CXFA_Para>(doc, packet);
    case XFA_Element::Part:
      return pdfium::MakeUnique<CXFA_Part>(doc, packet);
    case XFA_Element::Pdfa:
      return pdfium::MakeUnique<CXFA_Pdfa>(doc, packet);
    case XFA_Element::Filter:
      return pdfium::MakeUnique<CXFA_Filter>(doc, packet);
    case XFA_Element::Present:
      return pdfium::MakeUnique<CXFA_Present>(doc, packet);
    case XFA_Element::Pagination:
      return pdfium::MakeUnique<CXFA_Pagination>(doc, packet);
    case XFA_Element::Encoding:
      return pdfium::MakeUnique<CXFA_Encoding>(doc, packet);
    case XFA_Element::Event:
      return pdfium::MakeUnique<CXFA_Event>(doc, packet);
    case XFA_Element::Whitespace:
      return pdfium::MakeUnique<CXFA_Whitespace>(doc, packet);
    case XFA_Element::DefaultUi:
      return pdfium::MakeUnique<CXFA_DefaultUi>(doc, packet);
    case XFA_Element::DataModel:
      return pdfium::MakeUnique<CXFA_DataModel>(doc, packet);
    case XFA_Element::Barcode:
      return pdfium::MakeUnique<CXFA_Barcode>(doc, packet);
    case XFA_Element::TimePattern:
      return pdfium::MakeUnique<CXFA_TimePattern>(doc, packet);
    case XFA_Element::BatchOutput:
      return pdfium::MakeUnique<CXFA_BatchOutput>(doc, packet);
    case XFA_Element::Enforce:
      return pdfium::MakeUnique<CXFA_Enforce>(doc, packet);
    case XFA_Element::CurrencySymbols:
      return pdfium::MakeUnique<CXFA_CurrencySymbols>(doc, packet);
    case XFA_Element::AddSilentPrint:
      return pdfium::MakeUnique<CXFA_AddSilentPrint>(doc, packet);
    case XFA_Element::Rename:
      return pdfium::MakeUnique<CXFA_Rename>(doc, packet);
    case XFA_Element::Operation:
      return pdfium::MakeUnique<CXFA_Operation>(doc, packet);
    case XFA_Element::Typefaces:
      return pdfium::MakeUnique<CXFA_Typefaces>(doc, packet);
    case XFA_Element::SubjectDNs:
      return pdfium::MakeUnique<CXFA_SubjectDNs>(doc, packet);
    case XFA_Element::Issuers:
      return pdfium::MakeUnique<CXFA_Issuers>(doc, packet);
    case XFA_Element::SignaturePseudoModel:
      return pdfium::MakeUnique<CXFA_SignaturePseudoModel>(doc, packet);
    case XFA_Element::WsdlConnection:
      return pdfium::MakeUnique<CXFA_WsdlConnection>(doc, packet);
    case XFA_Element::Debug:
      return pdfium::MakeUnique<CXFA_Debug>(doc, packet);
    case XFA_Element::Delta:
      return pdfium::MakeUnique<CXFA_Delta>(doc, packet);
    case XFA_Element::EraNames:
      return pdfium::MakeUnique<CXFA_EraNames>(doc, packet);
    case XFA_Element::ModifyAnnots:
      return pdfium::MakeUnique<CXFA_ModifyAnnots>(doc, packet);
    case XFA_Element::StartNode:
      return pdfium::MakeUnique<CXFA_StartNode>(doc, packet);
    case XFA_Element::Button:
      return pdfium::MakeUnique<CXFA_Button>(doc, packet);
    case XFA_Element::Format:
      return pdfium::MakeUnique<CXFA_Format>(doc, packet);
    case XFA_Element::Border:
      return pdfium::MakeUnique<CXFA_Border>(doc, packet);
    case XFA_Element::Area:
      return pdfium::MakeUnique<CXFA_Area>(doc, packet);
    case XFA_Element::Hyphenation:
      return pdfium::MakeUnique<CXFA_Hyphenation>(doc, packet);
    case XFA_Element::Text:
      return pdfium::MakeUnique<CXFA_Text>(doc, packet);
    case XFA_Element::Time:
      return pdfium::MakeUnique<CXFA_Time>(doc, packet);
    case XFA_Element::Type:
      return pdfium::MakeUnique<CXFA_Type>(doc, packet);
    case XFA_Element::Overprint:
      return pdfium::MakeUnique<CXFA_Overprint>(doc, packet);
    case XFA_Element::Certificates:
      return pdfium::MakeUnique<CXFA_Certificates>(doc, packet);
    case XFA_Element::EncryptionMethods:
      return pdfium::MakeUnique<CXFA_EncryptionMethods>(doc, packet);
    case XFA_Element::SetProperty:
      return pdfium::MakeUnique<CXFA_SetProperty>(doc, packet);
    case XFA_Element::PrinterName:
      return pdfium::MakeUnique<CXFA_PrinterName>(doc, packet);
    case XFA_Element::StartPage:
      return pdfium::MakeUnique<CXFA_StartPage>(doc, packet);
    case XFA_Element::PageOffset:
      return pdfium::MakeUnique<CXFA_PageOffset>(doc, packet);
    case XFA_Element::DateTime:
      return pdfium::MakeUnique<CXFA_DateTime>(doc, packet);
    case XFA_Element::Comb:
      return pdfium::MakeUnique<CXFA_Comb>(doc, packet);
    case XFA_Element::Pattern:
      return pdfium::MakeUnique<CXFA_Pattern>(doc, packet);
    case XFA_Element::IfEmpty:
      return pdfium::MakeUnique<CXFA_IfEmpty>(doc, packet);
    case XFA_Element::SuppressBanner:
      return pdfium::MakeUnique<CXFA_SuppressBanner>(doc, packet);
    case XFA_Element::OutputBin:
      return pdfium::MakeUnique<CXFA_OutputBin>(doc, packet);
    case XFA_Element::Field:
      return pdfium::MakeUnique<CXFA_Field>(doc, packet);
    case XFA_Element::Agent:
      return pdfium::MakeUnique<CXFA_Agent>(doc, packet);
    case XFA_Element::OutputXSL:
      return pdfium::MakeUnique<CXFA_OutputXSL>(doc, packet);
    case XFA_Element::AdjustData:
      return pdfium::MakeUnique<CXFA_AdjustData>(doc, packet);
    case XFA_Element::AutoSave:
      return pdfium::MakeUnique<CXFA_AutoSave>(doc, packet);
    case XFA_Element::ContentArea:
      return pdfium::MakeUnique<CXFA_ContentArea>(doc, packet);
    case XFA_Element::EventPseudoModel:
      return pdfium::MakeUnique<CXFA_EventPseudoModel>(doc, packet);
    case XFA_Element::WsdlAddress:
      return pdfium::MakeUnique<CXFA_WsdlAddress>(doc, packet);
    case XFA_Element::Solid:
      return pdfium::MakeUnique<CXFA_Solid>(doc, packet);
    case XFA_Element::DateTimeSymbols:
      return pdfium::MakeUnique<CXFA_DateTimeSymbols>(doc, packet);
    case XFA_Element::EncryptionLevel:
      return pdfium::MakeUnique<CXFA_EncryptionLevel>(doc, packet);
    case XFA_Element::Edge:
      return pdfium::MakeUnique<CXFA_Edge>(doc, packet);
    case XFA_Element::Stipple:
      return pdfium::MakeUnique<CXFA_Stipple>(doc, packet);
    case XFA_Element::Attributes:
      return pdfium::MakeUnique<CXFA_Attributes>(doc, packet);
    case XFA_Element::VersionControl:
      return pdfium::MakeUnique<CXFA_VersionControl>(doc, packet);
    case XFA_Element::Meridiem:
      return pdfium::MakeUnique<CXFA_Meridiem>(doc, packet);
    case XFA_Element::ExclGroup:
      return pdfium::MakeUnique<CXFA_ExclGroup>(doc, packet);
    case XFA_Element::ToolTip:
      return pdfium::MakeUnique<CXFA_ToolTip>(doc, packet);
    case XFA_Element::Compress:
      return pdfium::MakeUnique<CXFA_Compress>(doc, packet);
    case XFA_Element::Reason:
      return pdfium::MakeUnique<CXFA_Reason>(doc, packet);
    case XFA_Element::Execute:
      return pdfium::MakeUnique<CXFA_Execute>(doc, packet);
    case XFA_Element::ContentCopy:
      return pdfium::MakeUnique<CXFA_ContentCopy>(doc, packet);
    case XFA_Element::DateTimeEdit:
      return pdfium::MakeUnique<CXFA_DateTimeEdit>(doc, packet);
    case XFA_Element::Config:
      return pdfium::MakeUnique<CXFA_Config>(doc, packet);
    case XFA_Element::Image:
      return pdfium::MakeUnique<CXFA_Image>(doc, packet);
    case XFA_Element::NumberOfCopies:
      return pdfium::MakeUnique<CXFA_NumberOfCopies>(doc, packet);
    case XFA_Element::BehaviorOverride:
      return pdfium::MakeUnique<CXFA_BehaviorOverride>(doc, packet);
    case XFA_Element::TimeStamp:
      return pdfium::MakeUnique<CXFA_TimeStamp>(doc, packet);
    case XFA_Element::Month:
      return pdfium::MakeUnique<CXFA_Month>(doc, packet);
    case XFA_Element::ViewerPreferences:
      return pdfium::MakeUnique<CXFA_ViewerPreferences>(doc, packet);
    case XFA_Element::ScriptModel:
      return pdfium::MakeUnique<CXFA_ScriptModel>(doc, packet);
    case XFA_Element::Decimal:
      return pdfium::MakeUnique<CXFA_Decimal>(doc, packet);
    case XFA_Element::Subform:
      return pdfium::MakeUnique<CXFA_Subform>(doc, packet);
    case XFA_Element::Select:
      return pdfium::MakeUnique<CXFA_Select>(doc, packet);
    case XFA_Element::Window:
      return pdfium::MakeUnique<CXFA_Window>(doc, packet);
    case XFA_Element::LocaleSet:
      return pdfium::MakeUnique<CXFA_LocaleSet>(doc, packet);
    case XFA_Element::Handler:
      return pdfium::MakeUnique<CXFA_Handler>(doc, packet);
    case XFA_Element::HostPseudoModel:
      return pdfium::MakeUnique<CXFA_HostPseudoModel>(doc, packet);
    case XFA_Element::Presence:
      return pdfium::MakeUnique<CXFA_Presence>(doc, packet);
    case XFA_Element::Record:
      return pdfium::MakeUnique<CXFA_Record>(doc, packet);
    case XFA_Element::Embed:
      return pdfium::MakeUnique<CXFA_Embed>(doc, packet);
    case XFA_Element::Version:
      return pdfium::MakeUnique<CXFA_Version>(doc, packet);
    case XFA_Element::Command:
      return pdfium::MakeUnique<CXFA_Command>(doc, packet);
    case XFA_Element::Copies:
      return pdfium::MakeUnique<CXFA_Copies>(doc, packet);
    case XFA_Element::Staple:
      return pdfium::MakeUnique<CXFA_Staple>(doc, packet);
    case XFA_Element::SubmitFormat:
      return pdfium::MakeUnique<CXFA_SubmitFormat>(doc, packet);
    case XFA_Element::Boolean:
      return pdfium::MakeUnique<CXFA_Boolean>(doc, packet);
    case XFA_Element::Message:
      return pdfium::MakeUnique<CXFA_Message>(doc, packet);
    case XFA_Element::Output:
      return pdfium::MakeUnique<CXFA_Output>(doc, packet);
    case XFA_Element::PsMap:
      return pdfium::MakeUnique<CXFA_PsMap>(doc, packet);
    case XFA_Element::ExcludeNS:
      return pdfium::MakeUnique<CXFA_ExcludeNS>(doc, packet);
    case XFA_Element::Assist:
      return pdfium::MakeUnique<CXFA_Assist>(doc, packet);
    case XFA_Element::Picture:
      return pdfium::MakeUnique<CXFA_Picture>(doc, packet);
    case XFA_Element::Traversal:
      return pdfium::MakeUnique<CXFA_Traversal>(doc, packet);
    case XFA_Element::SilentPrint:
      return pdfium::MakeUnique<CXFA_SilentPrint>(doc, packet);
    case XFA_Element::WebClient:
      return pdfium::MakeUnique<CXFA_WebClient>(doc, packet);
    case XFA_Element::LayoutPseudoModel:
      return pdfium::MakeUnique<CXFA_LayoutPseudoModel>(doc, packet);
    case XFA_Element::Producer:
      return pdfium::MakeUnique<CXFA_Producer>(doc, packet);
    case XFA_Element::Corner:
      return pdfium::MakeUnique<CXFA_Corner>(doc, packet);
    case XFA_Element::MsgId:
      return pdfium::MakeUnique<CXFA_MsgId>(doc, packet);
    case XFA_Element::Color:
      return pdfium::MakeUnique<CXFA_Color>(doc, packet);
    case XFA_Element::Keep:
      return pdfium::MakeUnique<CXFA_Keep>(doc, packet);
    case XFA_Element::Query:
      return pdfium::MakeUnique<CXFA_Query>(doc, packet);
    case XFA_Element::Insert:
      return pdfium::MakeUnique<CXFA_Insert>(doc, packet);
    case XFA_Element::ImageEdit:
      return pdfium::MakeUnique<CXFA_ImageEdit>(doc, packet);
    case XFA_Element::Validate:
      return pdfium::MakeUnique<CXFA_Validate>(doc, packet);
    case XFA_Element::DigestMethods:
      return pdfium::MakeUnique<CXFA_DigestMethods>(doc, packet);
    case XFA_Element::NumberPatterns:
      return pdfium::MakeUnique<CXFA_NumberPatterns>(doc, packet);
    case XFA_Element::PageSet:
      return pdfium::MakeUnique<CXFA_PageSet>(doc, packet);
    case XFA_Element::Integer:
      return pdfium::MakeUnique<CXFA_Integer>(doc, packet);
    case XFA_Element::SoapAddress:
      return pdfium::MakeUnique<CXFA_SoapAddress>(doc, packet);
    case XFA_Element::Equate:
      return pdfium::MakeUnique<CXFA_Equate>(doc, packet);
    case XFA_Element::FormFieldFilling:
      return pdfium::MakeUnique<CXFA_FormFieldFilling>(doc, packet);
    case XFA_Element::PageRange:
      return pdfium::MakeUnique<CXFA_PageRange>(doc, packet);
    case XFA_Element::Update:
      return pdfium::MakeUnique<CXFA_Update>(doc, packet);
    case XFA_Element::ConnectString:
      return pdfium::MakeUnique<CXFA_ConnectString>(doc, packet);
    case XFA_Element::Mode:
      return pdfium::MakeUnique<CXFA_Mode>(doc, packet);
    case XFA_Element::Layout:
      return pdfium::MakeUnique<CXFA_Layout>(doc, packet);
    case XFA_Element::XsdConnection:
      return pdfium::MakeUnique<CXFA_XsdConnection>(doc, packet);
    case XFA_Element::Traverse:
      return pdfium::MakeUnique<CXFA_Traverse>(doc, packet);
    case XFA_Element::Encodings:
      return pdfium::MakeUnique<CXFA_Encodings>(doc, packet);
    case XFA_Element::Template:
      return pdfium::MakeUnique<CXFA_Template>(doc, packet);
    case XFA_Element::Acrobat:
      return pdfium::MakeUnique<CXFA_Acrobat>(doc, packet);
    case XFA_Element::ValidationMessaging:
      return pdfium::MakeUnique<CXFA_ValidationMessaging>(doc, packet);
    case XFA_Element::Signing:
      return pdfium::MakeUnique<CXFA_Signing>(doc, packet);
    case XFA_Element::DataWindow:
      return pdfium::MakeUnique<CXFA_DataWindow>(doc, packet);
    case XFA_Element::Script:
      return pdfium::MakeUnique<CXFA_Script>(doc, packet);
    case XFA_Element::AddViewerPreferences:
      return pdfium::MakeUnique<CXFA_AddViewerPreferences>(doc, packet);
    case XFA_Element::AlwaysEmbed:
      return pdfium::MakeUnique<CXFA_AlwaysEmbed>(doc, packet);
    case XFA_Element::PasswordEdit:
      return pdfium::MakeUnique<CXFA_PasswordEdit>(doc, packet);
    case XFA_Element::NumericEdit:
      return pdfium::MakeUnique<CXFA_NumericEdit>(doc, packet);
    case XFA_Element::EncryptionMethod:
      return pdfium::MakeUnique<CXFA_EncryptionMethod>(doc, packet);
    case XFA_Element::Change:
      return pdfium::MakeUnique<CXFA_Change>(doc, packet);
    case XFA_Element::PageArea:
      return pdfium::MakeUnique<CXFA_PageArea>(doc, packet);
    case XFA_Element::SubmitUrl:
      return pdfium::MakeUnique<CXFA_SubmitUrl>(doc, packet);
    case XFA_Element::Oids:
      return pdfium::MakeUnique<CXFA_Oids>(doc, packet);
    case XFA_Element::Signature:
      return pdfium::MakeUnique<CXFA_Signature>(doc, packet);
    case XFA_Element::ADBE_JSConsole:
      return pdfium::MakeUnique<CXFA_aDBE_JSConsole>(doc, packet);
    case XFA_Element::Caption:
      return pdfium::MakeUnique<CXFA_Caption>(doc, packet);
    case XFA_Element::Relevant:
      return pdfium::MakeUnique<CXFA_Relevant>(doc, packet);
    case XFA_Element::FlipLabel:
      return pdfium::MakeUnique<CXFA_FlipLabel>(doc, packet);
    case XFA_Element::ExData:
      return pdfium::MakeUnique<CXFA_ExData>(doc, packet);
    case XFA_Element::DayNames:
      return pdfium::MakeUnique<CXFA_DayNames>(doc, packet);
    case XFA_Element::SoapAction:
      return pdfium::MakeUnique<CXFA_SoapAction>(doc, packet);
    case XFA_Element::DefaultTypeface:
      return pdfium::MakeUnique<CXFA_DefaultTypeface>(doc, packet);
    case XFA_Element::Manifest:
      return pdfium::MakeUnique<CXFA_Manifest>(doc, packet);
    case XFA_Element::Overflow:
      return pdfium::MakeUnique<CXFA_Overflow>(doc, packet);
    case XFA_Element::Linear:
      return pdfium::MakeUnique<CXFA_Linear>(doc, packet);
    case XFA_Element::CurrencySymbol:
      return pdfium::MakeUnique<CXFA_CurrencySymbol>(doc, packet);
    case XFA_Element::Delete:
      return pdfium::MakeUnique<CXFA_Delete>(doc, packet);
    case XFA_Element::Deltas:
      return pdfium::MakeUnique<CXFA_Deltas>(doc, packet);
    case XFA_Element::DigestMethod:
      return pdfium::MakeUnique<CXFA_DigestMethod>(doc, packet);
    case XFA_Element::InstanceManager:
      return pdfium::MakeUnique<CXFA_InstanceManager>(doc, packet);
    case XFA_Element::EquateRange:
      return pdfium::MakeUnique<CXFA_EquateRange>(doc, packet);
    case XFA_Element::Medium:
      return pdfium::MakeUnique<CXFA_Medium>(doc, packet);
    case XFA_Element::TextEdit:
      return pdfium::MakeUnique<CXFA_TextEdit>(doc, packet);
    case XFA_Element::TemplateCache:
      return pdfium::MakeUnique<CXFA_TemplateCache>(doc, packet);
    case XFA_Element::CompressObjectStream:
      return pdfium::MakeUnique<CXFA_CompressObjectStream>(doc, packet);
    case XFA_Element::DataValue:
      return pdfium::MakeUnique<CXFA_DataValue>(doc, packet);
    case XFA_Element::AccessibleContent:
      return pdfium::MakeUnique<CXFA_AccessibleContent>(doc, packet);
    case XFA_Element::IncludeXDPContent:
      return pdfium::MakeUnique<CXFA_IncludeXDPContent>(doc, packet);
    case XFA_Element::XmlConnection:
      return pdfium::MakeUnique<CXFA_XmlConnection>(doc, packet);
    case XFA_Element::ValidateApprovalSignatures:
      return pdfium::MakeUnique<CXFA_ValidateApprovalSignatures>(doc, packet);
    case XFA_Element::SignData:
      return pdfium::MakeUnique<CXFA_SignData>(doc, packet);
    case XFA_Element::Packets:
      return pdfium::MakeUnique<CXFA_Packets>(doc, packet);
    case XFA_Element::DatePattern:
      return pdfium::MakeUnique<CXFA_DatePattern>(doc, packet);
    case XFA_Element::DuplexOption:
      return pdfium::MakeUnique<CXFA_DuplexOption>(doc, packet);
    case XFA_Element::Base:
      return pdfium::MakeUnique<CXFA_Base>(doc, packet);
    case XFA_Element::Bind:
      return pdfium::MakeUnique<CXFA_Bind>(doc, packet);
    case XFA_Element::Compression:
      return pdfium::MakeUnique<CXFA_Compression>(doc, packet);
    case XFA_Element::User:
      return pdfium::MakeUnique<CXFA_User>(doc, packet);
    case XFA_Element::Rectangle:
      return pdfium::MakeUnique<CXFA_Rectangle>(doc, packet);
    case XFA_Element::EffectiveOutputPolicy:
      return pdfium::MakeUnique<CXFA_EffectiveOutputPolicy>(doc, packet);
    case XFA_Element::ADBE_JSDebugger:
      return pdfium::MakeUnique<CXFA_aDBE_JSDebugger>(doc, packet);
    case XFA_Element::Acrobat7:
      return pdfium::MakeUnique<CXFA_Acrobat7>(doc, packet);
    case XFA_Element::Interactive:
      return pdfium::MakeUnique<CXFA_Interactive>(doc, packet);
    case XFA_Element::Locale:
      return pdfium::MakeUnique<CXFA_Locale>(doc, packet);
    case XFA_Element::CurrentPage:
      return pdfium::MakeUnique<CXFA_CurrentPage>(doc, packet);
    case XFA_Element::Data:
      return pdfium::MakeUnique<CXFA_Data>(doc, packet);
    case XFA_Element::Date:
      return pdfium::MakeUnique<CXFA_Date>(doc, packet);
    case XFA_Element::Desc:
      return pdfium::MakeUnique<CXFA_Desc>(doc, packet);
    case XFA_Element::Encrypt:
      return pdfium::MakeUnique<CXFA_Encrypt>(doc, packet);
    case XFA_Element::Draw:
      return pdfium::MakeUnique<CXFA_Draw>(doc, packet);
    case XFA_Element::Encryption:
      return pdfium::MakeUnique<CXFA_Encryption>(doc, packet);
    case XFA_Element::MeridiemNames:
      return pdfium::MakeUnique<CXFA_MeridiemNames>(doc, packet);
    case XFA_Element::Messaging:
      return pdfium::MakeUnique<CXFA_Messaging>(doc, packet);
    case XFA_Element::Speak:
      return pdfium::MakeUnique<CXFA_Speak>(doc, packet);
    case XFA_Element::DataGroup:
      return pdfium::MakeUnique<CXFA_DataGroup>(doc, packet);
    case XFA_Element::Common:
      return pdfium::MakeUnique<CXFA_Common>(doc, packet);
    case XFA_Element::PaginationOverride:
      return pdfium::MakeUnique<CXFA_PaginationOverride>(doc, packet);
    case XFA_Element::Reasons:
      return pdfium::MakeUnique<CXFA_Reasons>(doc, packet);
    case XFA_Element::SignatureProperties:
      return pdfium::MakeUnique<CXFA_SignatureProperties>(doc, packet);
    case XFA_Element::Threshold:
      return pdfium::MakeUnique<CXFA_Threshold>(doc, packet);
    case XFA_Element::AppearanceFilter:
      return pdfium::MakeUnique<CXFA_AppearanceFilter>(doc, packet);
    case XFA_Element::Fill:
      return pdfium::MakeUnique<CXFA_Fill>(doc, packet);
    case XFA_Element::Font:
      return pdfium::MakeUnique<CXFA_Font>(doc, packet);
    case XFA_Element::Form:
      return pdfium::MakeUnique<CXFA_Form>(doc, packet);
    case XFA_Element::MediumInfo:
      return pdfium::MakeUnique<CXFA_MediumInfo>(doc, packet);
    case XFA_Element::Certificate:
      return pdfium::MakeUnique<CXFA_Certificate>(doc, packet);
    case XFA_Element::Password:
      return pdfium::MakeUnique<CXFA_Password>(doc, packet);
    case XFA_Element::RunScripts:
      return pdfium::MakeUnique<CXFA_RunScripts>(doc, packet);
    case XFA_Element::Trace:
      return pdfium::MakeUnique<CXFA_Trace>(doc, packet);
    case XFA_Element::Float:
      return pdfium::MakeUnique<CXFA_Float>(doc, packet);
    case XFA_Element::RenderPolicy:
      return pdfium::MakeUnique<CXFA_RenderPolicy>(doc, packet);
    case XFA_Element::LogPseudoModel:
      return pdfium::MakeUnique<CXFA_LogPseudoModel>(doc, packet);
    case XFA_Element::Destination:
      return pdfium::MakeUnique<CXFA_Destination>(doc, packet);
    case XFA_Element::Value:
      return pdfium::MakeUnique<CXFA_Value>(doc, packet);
    case XFA_Element::Bookend:
      return pdfium::MakeUnique<CXFA_Bookend>(doc, packet);
    case XFA_Element::ExObject:
      return pdfium::MakeUnique<CXFA_ExObject>(doc, packet);
    case XFA_Element::OpenAction:
      return pdfium::MakeUnique<CXFA_OpenAction>(doc, packet);
    case XFA_Element::NeverEmbed:
      return pdfium::MakeUnique<CXFA_NeverEmbed>(doc, packet);
    case XFA_Element::BindItems:
      return pdfium::MakeUnique<CXFA_BindItems>(doc, packet);
    case XFA_Element::Calculate:
      return pdfium::MakeUnique<CXFA_Calculate>(doc, packet);
    case XFA_Element::Print:
      return pdfium::MakeUnique<CXFA_Print>(doc, packet);
    case XFA_Element::Extras:
      return pdfium::MakeUnique<CXFA_Extras>(doc, packet);
    case XFA_Element::Proto:
      return pdfium::MakeUnique<CXFA_Proto>(doc, packet);
    case XFA_Element::DSigData:
      return pdfium::MakeUnique<CXFA_DSigData>(doc, packet);
    case XFA_Element::Creator:
      return pdfium::MakeUnique<CXFA_Creator>(doc, packet);
    case XFA_Element::Connect:
      return pdfium::MakeUnique<CXFA_Connect>(doc, packet);
    case XFA_Element::Permissions:
      return pdfium::MakeUnique<CXFA_Permissions>(doc, packet);
    case XFA_Element::ConnectionSet:
      return pdfium::MakeUnique<CXFA_ConnectionSet>(doc, packet);
    case XFA_Element::Submit:
      return pdfium::MakeUnique<CXFA_Submit>(doc, packet);
    case XFA_Element::Range:
      return pdfium::MakeUnique<CXFA_Range>(doc, packet);
    case XFA_Element::Linearized:
      return pdfium::MakeUnique<CXFA_Linearized>(doc, packet);
    case XFA_Element::Packet:
      return pdfium::MakeUnique<CXFA_Packet>(doc, packet);
    case XFA_Element::RootElement:
      return pdfium::MakeUnique<CXFA_RootElement>(doc, packet);
    case XFA_Element::PlaintextMetadata:
      return pdfium::MakeUnique<CXFA_PlaintextMetadata>(doc, packet);
    case XFA_Element::NumberSymbols:
      return pdfium::MakeUnique<CXFA_NumberSymbols>(doc, packet);
    case XFA_Element::PrintHighQuality:
      return pdfium::MakeUnique<CXFA_PrintHighQuality>(doc, packet);
    case XFA_Element::Driver:
      return pdfium::MakeUnique<CXFA_Driver>(doc, packet);
    case XFA_Element::IncrementalLoad:
      return pdfium::MakeUnique<CXFA_IncrementalLoad>(doc, packet);
    case XFA_Element::SubjectDN:
      return pdfium::MakeUnique<CXFA_SubjectDN>(doc, packet);
    case XFA_Element::CompressLogicalStructure:
      return pdfium::MakeUnique<CXFA_CompressLogicalStructure>(doc, packet);
    case XFA_Element::IncrementalMerge:
      return pdfium::MakeUnique<CXFA_IncrementalMerge>(doc, packet);
    case XFA_Element::Radial:
      return pdfium::MakeUnique<CXFA_Radial>(doc, packet);
    case XFA_Element::Variables:
      return pdfium::MakeUnique<CXFA_Variables>(doc, packet);
    case XFA_Element::TimePatterns:
      return pdfium::MakeUnique<CXFA_TimePatterns>(doc, packet);
    case XFA_Element::EffectiveInputPolicy:
      return pdfium::MakeUnique<CXFA_EffectiveInputPolicy>(doc, packet);
    case XFA_Element::NameAttr:
      return pdfium::MakeUnique<CXFA_NameAttr>(doc, packet);
    case XFA_Element::Conformance:
      return pdfium::MakeUnique<CXFA_Conformance>(doc, packet);
    case XFA_Element::Transform:
      return pdfium::MakeUnique<CXFA_Transform>(doc, packet);
    case XFA_Element::LockDocument:
      return pdfium::MakeUnique<CXFA_LockDocument>(doc, packet);
    case XFA_Element::BreakAfter:
      return pdfium::MakeUnique<CXFA_BreakAfter>(doc, packet);
    case XFA_Element::Line:
      return pdfium::MakeUnique<CXFA_Line>(doc, packet);
    case XFA_Element::List:
      return pdfium::MakeUnique<CXFA_List>(doc, packet);
    case XFA_Element::Source:
      return pdfium::MakeUnique<CXFA_Source>(doc, packet);
    case XFA_Element::Occur:
      return pdfium::MakeUnique<CXFA_Occur>(doc, packet);
    case XFA_Element::PickTrayByPDFSize:
      return pdfium::MakeUnique<CXFA_PickTrayByPDFSize>(doc, packet);
    case XFA_Element::MonthNames:
      return pdfium::MakeUnique<CXFA_MonthNames>(doc, packet);
    case XFA_Element::Severity:
      return pdfium::MakeUnique<CXFA_Severity>(doc, packet);
    case XFA_Element::GroupParent:
      return pdfium::MakeUnique<CXFA_GroupParent>(doc, packet);
    case XFA_Element::DocumentAssembly:
      return pdfium::MakeUnique<CXFA_DocumentAssembly>(doc, packet);
    case XFA_Element::NumberSymbol:
      return pdfium::MakeUnique<CXFA_NumberSymbol>(doc, packet);
    case XFA_Element::Tagged:
      return pdfium::MakeUnique<CXFA_Tagged>(doc, packet);
    case XFA_Element::Items:
      return pdfium::MakeUnique<CXFA_Items>(doc, packet);
    default:
      break;
  }

  return std::unique_ptr<CXFA_Node>(new CXFA_Node(
      doc, packet, pElement->eObjectType, pElement->eName, pElement->pName));
}

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     uint16_t ePacket,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     const WideStringView& elementName)
    : CXFA_Object(pDoc,
                  oType,
                  eType,
                  elementName,
                  pdfium::MakeUnique<CJX_Node>(this)),
      m_pNext(nullptr),
      m_pChild(nullptr),
      m_pLastChild(nullptr),
      m_pParent(nullptr),
      m_pXMLNode(nullptr),
      m_ePacket(ePacket),
      m_uNodeFlags(XFA_NodeFlag_None),
      m_dwNameHash(0),
      m_pAuxNode(nullptr) {
  ASSERT(m_pDocument);
}

CXFA_Node::~CXFA_Node() {
  ASSERT(!m_pParent);
  CXFA_Node* pNode = m_pChild;
  while (pNode) {
    CXFA_Node* pNext = pNode->m_pNext;
    pNode->m_pParent = nullptr;
    delete pNode;
    pNode = pNext;
  }
  if (m_pXMLNode && IsOwnXMLNode())
    delete m_pXMLNode;
}

CXFA_Node* CXFA_Node::Clone(bool bRecursive) {
  CXFA_Node* pClone = m_pDocument->CreateNode(m_ePacket, m_elementType);
  if (!pClone)
    return nullptr;

  JSNode()->MergeAllData(pClone);
  pClone->UpdateNameHash();
  if (IsNeedSavingXMLNode()) {
    std::unique_ptr<CFX_XMLNode> pCloneXML;
    if (IsAttributeInXML()) {
      WideString wsName;
      JSNode()->GetAttribute(XFA_Attribute::Name, wsName, false);
      auto pCloneXMLElement = pdfium::MakeUnique<CFX_XMLElement>(wsName);
      WideStringView wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty()) {
        pCloneXMLElement->SetTextData(WideString(wsValue));
      }
      pCloneXML.reset(pCloneXMLElement.release());
      pClone->JSNode()->SetEnum(XFA_Attribute::Contains,
                                XFA_ATTRIBUTEENUM_Unknown, false);
    } else {
      pCloneXML = m_pXMLNode->Clone();
    }
    pClone->SetXMLMappingNode(pCloneXML.release());
    pClone->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->Clone(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  pClone->JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr, nullptr);
  return pClone;
}

CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem) const {
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      return m_pNext;
    case XFA_NODEITEM_FirstChild:
      return m_pChild;
    case XFA_NODEITEM_Parent:
      return m_pParent;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        CXFA_Node* pPrev = nullptr;
        while (pSibling && pSibling != this) {
          pPrev = pSibling;
          pSibling = pSibling->m_pNext;
        }
        return pPrev;
      }
      return nullptr;
    default:
      break;
  }
  return nullptr;
}

WideStringView CXFA_Node::GetName() const {
  return GetClassName();
}

CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem,
                                  XFA_ObjectType eType) const {
  CXFA_Node* pNode = nullptr;
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      pNode = m_pNext;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pNext;
      break;
    case XFA_NODEITEM_FirstChild:
      pNode = m_pChild;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pNext;
      break;
    case XFA_NODEITEM_Parent:
      pNode = m_pParent;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pParent;
      break;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        while (pSibling && pSibling != this) {
          if (eType == pSibling->GetObjectType())
            pNode = pSibling;

          pSibling = pSibling->m_pNext;
        }
      }
      break;
    default:
      break;
  }
  return pNode;
}

std::vector<CXFA_Node*> CXFA_Node::GetNodeList(uint32_t dwTypeFilter,
                                               XFA_Element eTypeFilter) {
  std::vector<CXFA_Node*> nodes;
  if (eTypeFilter != XFA_Element::Unknown) {
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext) {
      if (pChild->GetElementType() == eTypeFilter)
        nodes.push_back(pChild);
    }
  } else if (dwTypeFilter ==
             (XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties)) {
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext)
      nodes.push_back(pChild);
  } else if (dwTypeFilter != 0) {
    bool bFilterChildren = !!(dwTypeFilter & XFA_NODEFILTER_Children);
    bool bFilterProperties = !!(dwTypeFilter & XFA_NODEFILTER_Properties);
    bool bFilterOneOfProperties =
        !!(dwTypeFilter & XFA_NODEFILTER_OneOfProperty);
    CXFA_Node* pChild = m_pChild;
    while (pChild) {
      const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
          GetElementType(), pChild->GetElementType(), XFA_XDPPACKET_UNKNOWN);
      if (pProperty) {
        if (bFilterProperties) {
          nodes.push_back(pChild);
        } else if (bFilterOneOfProperties &&
                   (pProperty->uFlags & XFA_PROPERTYFLAG_OneOf)) {
          nodes.push_back(pChild);
        } else if (bFilterChildren &&
                   (pChild->GetElementType() == XFA_Element::Variables ||
                    pChild->GetElementType() == XFA_Element::PageSet)) {
          nodes.push_back(pChild);
        }
      } else if (bFilterChildren) {
        nodes.push_back(pChild);
      }
      pChild = pChild->m_pNext;
    }
    if (bFilterOneOfProperties && nodes.empty()) {
      int32_t iProperties = 0;
      const XFA_PROPERTY* pProperty =
          XFA_GetElementProperties(GetElementType(), iProperties);
      if (!pProperty || iProperties < 1)
        return nodes;
      for (int32_t i = 0; i < iProperties; i++) {
        if (pProperty[i].uFlags & XFA_PROPERTYFLAG_DefaultOneOf) {
          const XFA_PACKETINFO* pPacket = XFA_GetPacketByID(GetPacketID());
          CXFA_Node* pNewNode =
              m_pDocument->CreateNode(pPacket, pProperty[i].eName);
          if (!pNewNode)
            break;
          InsertChild(pNewNode, nullptr);
          pNewNode->SetFlag(XFA_NodeFlag_Initialized, true);
          nodes.push_back(pNewNode);
          break;
        }
      }
    }
  }
  return nodes;
}

CXFA_Node* CXFA_Node::CreateSamePacketNode(XFA_Element eType) {
  CXFA_Node* pNode = m_pDocument->CreateNode(m_ePacket, eType);
  pNode->SetFlag(XFA_NodeFlag_Initialized, true);
  return pNode;
}

CXFA_Node* CXFA_Node::CloneTemplateToForm(bool bRecursive) {
  ASSERT(m_ePacket == XFA_XDPPACKET_Template);
  CXFA_Node* pClone =
      m_pDocument->CreateNode(XFA_XDPPACKET_Form, m_elementType);
  if (!pClone)
    return nullptr;

  pClone->SetTemplateNode(this);
  pClone->UpdateNameHash();
  pClone->SetXMLMappingNode(GetXMLMappingNode());
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->CloneTemplateToForm(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  return pClone;
}

CXFA_Node* CXFA_Node::GetTemplateNode() const {
  return m_pAuxNode;
}

void CXFA_Node::SetTemplateNode(CXFA_Node* pTemplateNode) {
  m_pAuxNode = pTemplateNode;
}

CXFA_Node* CXFA_Node::GetBindData() {
  ASSERT(GetPacketID() == XFA_XDPPACKET_Form);
  return static_cast<CXFA_Node*>(
      JSNode()->GetObject(XFA_Attribute::BindingNode));
}

std::vector<CXFA_Node*> CXFA_Node::GetBindItems() {
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    return *static_cast<std::vector<CXFA_Node*>*>(pBinding);
  }
  std::vector<CXFA_Node*> result;
  CXFA_Node* pFormNode =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (pFormNode)
    result.push_back(pFormNode);
  return result;
}

int32_t CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  ASSERT(pFormNode);
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    auto* pItems = static_cast<std::vector<CXFA_Node*>*>(pBinding);
    if (!pdfium::ContainsValue(*pItems, pFormNode))
      pItems->push_back(pFormNode);
    return pdfium::CollectionSize<int32_t>(*pItems);
  }
  CXFA_Node* pOldFormItem =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (!pOldFormItem) {
    JSNode()->SetObject(XFA_Attribute::BindingNode, pFormNode, nullptr);
    return 1;
  }
  if (pOldFormItem == pFormNode)
    return 1;

  std::vector<CXFA_Node*>* pItems = new std::vector<CXFA_Node*>;
  JSNode()->SetObject(XFA_Attribute::BindingNode, pItems,
                      &deleteBindItemCallBack);
  pItems->push_back(pOldFormItem);
  pItems->push_back(pFormNode);
  m_uNodeFlags |= XFA_NodeFlag_BindFormItems;
  return 2;
}

int32_t CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    auto* pItems = static_cast<std::vector<CXFA_Node*>*>(pBinding);
    auto iter = std::find(pItems->begin(), pItems->end(), pFormNode);
    if (iter != pItems->end()) {
      *iter = pItems->back();
      pItems->pop_back();
      if (pItems->size() == 1) {
        JSNode()->SetObject(XFA_Attribute::BindingNode, (*pItems)[0],
                            nullptr);  // Invalidates pItems.
        m_uNodeFlags &= ~XFA_NodeFlag_BindFormItems;
        return 1;
      }
    }
    return pdfium::CollectionSize<int32_t>(*pItems);
  }
  CXFA_Node* pOldFormItem =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (pOldFormItem != pFormNode)
    return pOldFormItem ? 1 : 0;

  JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr, nullptr);
  return 0;
}

bool CXFA_Node::HasBindItem() {
  return GetPacketID() == XFA_XDPPACKET_Datasets &&
         JSNode()->GetObject(XFA_Attribute::BindingNode);
}

CXFA_WidgetData* CXFA_Node::GetWidgetData() {
  return (CXFA_WidgetData*)JSNode()->GetObject(XFA_Attribute::WidgetData);
}

CXFA_WidgetData* CXFA_Node::GetContainerWidgetData() {
  if (GetPacketID() != XFA_XDPPACKET_Form)
    return nullptr;
  XFA_Element eType = GetElementType();
  if (eType == XFA_Element::ExclGroup)
    return nullptr;
  CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::ExclGroup)
    return nullptr;

  if (eType == XFA_Element::Field) {
    CXFA_WidgetData* pFieldWidgetData = GetWidgetData();
    if (pFieldWidgetData &&
        pFieldWidgetData->GetChoiceListOpen() ==
            XFA_ATTRIBUTEENUM_MultiSelect) {
      return nullptr;
    } else {
      WideString wsPicture;
      if (pFieldWidgetData) {
        pFieldWidgetData->GetPictureContent(wsPicture,
                                            XFA_VALUEPICTURE_DataBind);
      }
      if (!wsPicture.IsEmpty())
        return pFieldWidgetData;
      CXFA_Node* pDataNode = GetBindData();
      if (!pDataNode)
        return nullptr;
      pFieldWidgetData = nullptr;
      for (CXFA_Node* pFormNode : pDataNode->GetBindItems()) {
        if (!pFormNode || pFormNode->HasRemovedChildren())
          continue;
        pFieldWidgetData = pFormNode->GetWidgetData();
        if (pFieldWidgetData) {
          pFieldWidgetData->GetPictureContent(wsPicture,
                                              XFA_VALUEPICTURE_DataBind);
        }
        if (!wsPicture.IsEmpty())
          break;
        pFieldWidgetData = nullptr;
      }
      return pFieldWidgetData;
    }
  }
  CXFA_Node* pGrandNode =
      pParentNode ? pParentNode->GetNodeItem(XFA_NODEITEM_Parent) : nullptr;
  CXFA_Node* pValueNode =
      (pParentNode && pParentNode->GetElementType() == XFA_Element::Value)
          ? pParentNode
          : nullptr;
  if (!pValueNode) {
    pValueNode =
        (pGrandNode && pGrandNode->GetElementType() == XFA_Element::Value)
            ? pGrandNode
            : nullptr;
  }
  CXFA_Node* pParentOfValueNode =
      pValueNode ? pValueNode->GetNodeItem(XFA_NODEITEM_Parent) : nullptr;
  return pParentOfValueNode ? pParentOfValueNode->GetContainerWidgetData()
                            : nullptr;
}

bool CXFA_Node::GetLocaleName(WideString& wsLocaleName) {
  CXFA_Node* pForm = GetDocument()->GetXFAObject(XFA_HASHCODE_Form)->AsNode();
  CXFA_Node* pTopSubform = pForm->GetFirstChildByClass(XFA_Element::Subform);
  ASSERT(pTopSubform);
  CXFA_Node* pLocaleNode = this;
  bool bLocale = false;
  do {
    bLocale = pLocaleNode->JSNode()->TryCData(XFA_Attribute::Locale,
                                              wsLocaleName, false);
    if (!bLocale) {
      pLocaleNode = pLocaleNode->GetNodeItem(XFA_NODEITEM_Parent);
    }
  } while (pLocaleNode && pLocaleNode != pTopSubform && !bLocale);
  if (bLocale)
    return true;
  CXFA_Node* pConfig = ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Config));
  wsLocaleName = GetDocument()->GetLocalMgr()->GetConfigLocaleName(pConfig);
  if (!wsLocaleName.IsEmpty())
    return true;
  if (pTopSubform && pTopSubform->JSNode()->TryCData(XFA_Attribute::Locale,
                                                     wsLocaleName, false)) {
    return true;
  }
  IFX_Locale* pLocale = GetDocument()->GetLocalMgr()->GetDefLocale();
  if (pLocale) {
    wsLocaleName = pLocale->GetName();
    return true;
  }
  return false;
}

XFA_ATTRIBUTEENUM CXFA_Node::GetIntact() {
  CXFA_Node* pKeep = GetFirstChildByClass(XFA_Element::Keep);
  XFA_ATTRIBUTEENUM eLayoutType = JSNode()->GetEnum(XFA_Attribute::Layout);
  if (pKeep) {
    XFA_ATTRIBUTEENUM eIntact;
    if (pKeep->JSNode()->TryEnum(XFA_Attribute::Intact, eIntact, false)) {
      if (eIntact == XFA_ATTRIBUTEENUM_None &&
          eLayoutType == XFA_ATTRIBUTEENUM_Row &&
          m_pDocument->GetCurVersionMode() < XFA_VERSION_208) {
        CXFA_Node* pPreviewRow = GetNodeItem(XFA_NODEITEM_PrevSibling,
                                             XFA_ObjectType::ContainerNode);
        if (pPreviewRow &&
            pPreviewRow->JSNode()->GetEnum(XFA_Attribute::Layout) ==
                XFA_ATTRIBUTEENUM_Row) {
          XFA_ATTRIBUTEENUM eValue;
          if (pKeep->JSNode()->TryEnum(XFA_Attribute::Previous, eValue,
                                       false) &&
              (eValue == XFA_ATTRIBUTEENUM_ContentArea ||
               eValue == XFA_ATTRIBUTEENUM_PageArea)) {
            return XFA_ATTRIBUTEENUM_ContentArea;
          }
          CXFA_Node* pNode =
              pPreviewRow->GetFirstChildByClass(XFA_Element::Keep);
          if (pNode &&
              pNode->JSNode()->TryEnum(XFA_Attribute::Next, eValue, false) &&
              (eValue == XFA_ATTRIBUTEENUM_ContentArea ||
               eValue == XFA_ATTRIBUTEENUM_PageArea)) {
            return XFA_ATTRIBUTEENUM_ContentArea;
          }
        }
      }
      return eIntact;
    }
  }
  switch (GetElementType()) {
    case XFA_Element::Subform:
      switch (eLayoutType) {
        case XFA_ATTRIBUTEENUM_Position:
        case XFA_ATTRIBUTEENUM_Row:
          return XFA_ATTRIBUTEENUM_ContentArea;
        case XFA_ATTRIBUTEENUM_Tb:
        case XFA_ATTRIBUTEENUM_Table:
        case XFA_ATTRIBUTEENUM_Lr_tb:
        case XFA_ATTRIBUTEENUM_Rl_tb:
          return XFA_ATTRIBUTEENUM_None;
        default:
          break;
      }
      break;
    case XFA_Element::Field: {
      CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
      if (!pParentNode ||
          pParentNode->GetElementType() == XFA_Element::PageArea)
        return XFA_ATTRIBUTEENUM_ContentArea;
      if (pParentNode->GetIntact() == XFA_ATTRIBUTEENUM_None) {
        XFA_ATTRIBUTEENUM eParLayout =
            pParentNode->JSNode()->GetEnum(XFA_Attribute::Layout);
        if (eParLayout == XFA_ATTRIBUTEENUM_Position ||
            eParLayout == XFA_ATTRIBUTEENUM_Row ||
            eParLayout == XFA_ATTRIBUTEENUM_Table) {
          return XFA_ATTRIBUTEENUM_None;
        }
        XFA_VERSION version = m_pDocument->GetCurVersionMode();
        if (eParLayout == XFA_ATTRIBUTEENUM_Tb && version < XFA_VERSION_208) {
          CXFA_Measurement measureH;
          if (JSNode()->TryMeasure(XFA_Attribute::H, measureH, false))
            return XFA_ATTRIBUTEENUM_ContentArea;
        }
        return XFA_ATTRIBUTEENUM_None;
      }
      return XFA_ATTRIBUTEENUM_ContentArea;
    }
    case XFA_Element::Draw:
      return XFA_ATTRIBUTEENUM_ContentArea;
    default:
      break;
  }
  return XFA_ATTRIBUTEENUM_None;
}

CXFA_Node* CXFA_Node::GetDataDescriptionNode() {
  if (m_ePacket == XFA_XDPPACKET_Datasets)
    return m_pAuxNode;
  return nullptr;
}

void CXFA_Node::SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode) {
  ASSERT(m_ePacket == XFA_XDPPACKET_Datasets);
  m_pAuxNode = pDataDescriptionNode;
}

CXFA_Node* CXFA_Node::GetModelNode() {
  switch (GetPacketID()) {
    case XFA_XDPPACKET_XDP:
      return m_pDocument->GetRoot();
    case XFA_XDPPACKET_Config:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Config));
    case XFA_XDPPACKET_Template:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Template));
    case XFA_XDPPACKET_Form:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Form));
    case XFA_XDPPACKET_Datasets:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Datasets));
    case XFA_XDPPACKET_LocaleSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_LocaleSet));
    case XFA_XDPPACKET_ConnectionSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_ConnectionSet));
    case XFA_XDPPACKET_SourceSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_SourceSet));
    case XFA_XDPPACKET_Xdc:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Xdc));
    default:
      return this;
  }
}

int32_t CXFA_Node::CountChildren(XFA_Element eType, bool bOnlyChild) {
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType || eType == XFA_Element::Unknown) {
      if (bOnlyChild) {
        const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
            GetElementType(), pNode->GetElementType(), XFA_XDPPACKET_UNKNOWN);
        if (pProperty) {
          continue;
        }
      }
      iCount++;
    }
  }
  return iCount;
}

CXFA_Node* CXFA_Node::GetChild(int32_t index,
                               XFA_Element eType,
                               bool bOnlyChild) {
  ASSERT(index > -1);
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType || eType == XFA_Element::Unknown) {
      if (bOnlyChild) {
        const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
            GetElementType(), pNode->GetElementType(), XFA_XDPPACKET_UNKNOWN);
        if (pProperty) {
          continue;
        }
      }
      iCount++;
      if (iCount > index) {
        return pNode;
      }
    }
  }
  return nullptr;
}

int32_t CXFA_Node::InsertChild(int32_t index, CXFA_Node* pNode) {
  ASSERT(!pNode->m_pNext);
  pNode->m_pParent = this;
  bool ret = m_pDocument->RemovePurgeNode(pNode);
  ASSERT(ret);
  (void)ret;  // Avoid unused variable warning.

  if (!m_pChild || index == 0) {
    if (index > 0) {
      return -1;
    }
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    index = 0;
  } else if (index < 0) {
    m_pLastChild->m_pNext = pNode;
  } else {
    CXFA_Node* pPrev = m_pChild;
    int32_t iCount = 0;
    while (++iCount != index && pPrev->m_pNext) {
      pPrev = pPrev->m_pNext;
    }
    if (index > 0 && index != iCount) {
      return -1;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
    index = iCount;
  }
  if (!pNode->m_pNext) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild);
  ASSERT(!m_pLastChild->m_pNext);
  pNode->ClearFlag(XFA_NodeFlag_HasRemovedChildren);
  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildAdded(this);

  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    ASSERT(!pNode->m_pXMLNode->GetNodeItem(CFX_XMLNode::Parent));
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, index);
    pNode->ClearFlag(XFA_NodeFlag_OwnXMLNode);
  }
  return index;
}

bool CXFA_Node::InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode) {
  if (!pNode || pNode->m_pParent ||
      (pBeforeNode && pBeforeNode->m_pParent != this)) {
    NOTREACHED();
    return false;
  }
  bool ret = m_pDocument->RemovePurgeNode(pNode);
  ASSERT(ret);
  (void)ret;  // Avoid unused variable warning.

  int32_t nIndex = -1;
  pNode->m_pParent = this;
  if (!m_pChild || pBeforeNode == m_pChild) {
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    nIndex = 0;
  } else if (!pBeforeNode) {
    pNode->m_pNext = m_pLastChild->m_pNext;
    m_pLastChild->m_pNext = pNode;
  } else {
    nIndex = 1;
    CXFA_Node* pPrev = m_pChild;
    while (pPrev->m_pNext != pBeforeNode) {
      pPrev = pPrev->m_pNext;
      nIndex++;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
  }
  if (!pNode->m_pNext) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild);
  ASSERT(!m_pLastChild->m_pNext);
  pNode->ClearFlag(XFA_NodeFlag_HasRemovedChildren);
  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildAdded(this);

  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    ASSERT(!pNode->m_pXMLNode->GetNodeItem(CFX_XMLNode::Parent));
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, nIndex);
    pNode->ClearFlag(XFA_NodeFlag_OwnXMLNode);
  }
  return true;
}

CXFA_Node* CXFA_Node::Deprecated_GetPrevSibling() {
  if (!m_pParent) {
    return nullptr;
  }
  for (CXFA_Node* pSibling = m_pParent->m_pChild; pSibling;
       pSibling = pSibling->m_pNext) {
    if (pSibling->m_pNext == this) {
      return pSibling;
    }
  }
  return nullptr;
}

bool CXFA_Node::RemoveChild(CXFA_Node* pNode, bool bNotify) {
  if (!pNode || pNode->m_pParent != this) {
    NOTREACHED();
    return false;
  }
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext;
    }
    pNode->m_pNext = nullptr;
    pNode->m_pParent = nullptr;
  } else {
    CXFA_Node* pPrev = pNode->Deprecated_GetPrevSibling();
    pPrev->m_pNext = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext ? pNode->m_pNext : pPrev;
    }
    pNode->m_pNext = nullptr;
    pNode->m_pParent = nullptr;
  }
  ASSERT(!m_pLastChild || !m_pLastChild->m_pNext);
  OnRemoved(bNotify);
  pNode->SetFlag(XFA_NodeFlag_HasRemovedChildren, true);
  m_pDocument->AddPurgeNode(pNode);
  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    if (pNode->IsAttributeInXML()) {
      ASSERT(pNode->m_pXMLNode == m_pXMLNode &&
             m_pXMLNode->GetType() == FX_XMLNODE_Element);
      if (pNode->m_pXMLNode->GetType() == FX_XMLNODE_Element) {
        CFX_XMLElement* pXMLElement =
            static_cast<CFX_XMLElement*>(pNode->m_pXMLNode);
        WideStringView wsAttributeName =
            pNode->JSNode()->GetCData(XFA_Attribute::QualifiedName);
        // TODO(tsepez): check usage of c_str() below.
        pXMLElement->RemoveAttribute(wsAttributeName.unterminated_c_str());
      }
      WideString wsName;
      pNode->JSNode()->GetAttribute(XFA_Attribute::Name, wsName, false);
      CFX_XMLElement* pNewXMLElement = new CFX_XMLElement(wsName);
      WideStringView wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty()) {
        pNewXMLElement->SetTextData(WideString(wsValue));
      }
      pNode->m_pXMLNode = pNewXMLElement;
      pNode->JSNode()->SetEnum(XFA_Attribute::Contains,
                               XFA_ATTRIBUTEENUM_Unknown, false);
    } else {
      m_pXMLNode->RemoveChildNode(pNode->m_pXMLNode);
    }
    pNode->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  return true;
}

CXFA_Node* CXFA_Node::GetFirstChildByName(const WideStringView& wsName) const {
  return GetFirstChildByName(FX_HashCode_GetW(wsName, false));
}

CXFA_Node* CXFA_Node::GetFirstChildByName(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstChildByClass(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(
    const WideStringView& wsNodeName) const {
  return GetNextSameNameSibling(FX_HashCode_GetW(wsNodeName, false));
}

CXFA_Node* CXFA_Node::GetNextSameClassSibling(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType) {
      return pNode;
    }
  }
  return nullptr;
}

int32_t CXFA_Node::GetNodeSameNameIndex() const {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByName(const_cast<CXFA_Node*>(this));
}

int32_t CXFA_Node::GetNodeSameClassIndex() const {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByClassName(const_cast<CXFA_Node*>(this));
}

void CXFA_Node::GetSOMExpression(WideString& wsSOMExpression) {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  pScriptContext->GetSomExpression(this, wsSOMExpression);
}

CXFA_Node* CXFA_Node::GetInstanceMgrOfSubform() {
  CXFA_Node* pInstanceMgr = nullptr;
  if (m_ePacket == XFA_XDPPACKET_Form) {
    CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
    if (!pParentNode || pParentNode->GetElementType() == XFA_Element::Area) {
      return pInstanceMgr;
    }
    for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling); pNode;
         pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
      XFA_Element eType = pNode->GetElementType();
      if ((eType == XFA_Element::Subform || eType == XFA_Element::SubformSet) &&
          pNode->m_dwNameHash != m_dwNameHash) {
        break;
      }
      if (eType == XFA_Element::InstanceManager) {
        WideStringView wsName = JSNode()->GetCData(XFA_Attribute::Name);
        WideStringView wsInstName =
            pNode->JSNode()->GetCData(XFA_Attribute::Name);
        if (wsInstName.GetLength() > 0 && wsInstName[0] == '_' &&
            wsInstName.Right(wsInstName.GetLength() - 1) == wsName) {
          pInstanceMgr = pNode;
        }
        break;
      }
    }
  }
  return pInstanceMgr;
}

CXFA_Node* CXFA_Node::GetOccurNode() {
  return GetFirstChildByClass(XFA_Element::Occur);
}

bool CXFA_Node::HasFlag(XFA_NodeFlag dwFlag) const {
  if (m_uNodeFlags & dwFlag)
    return true;
  if (dwFlag == XFA_NodeFlag_HasRemovedChildren)
    return m_pParent && m_pParent->HasFlag(dwFlag);
  return false;
}

void CXFA_Node::SetFlag(uint32_t dwFlag, bool bNotify) {
  if (dwFlag == XFA_NodeFlag_Initialized && bNotify && !IsInitialized()) {
    CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
    if (pNotify) {
      pNotify->OnNodeReady(this);
    }
  }
  m_uNodeFlags |= dwFlag;
}

void CXFA_Node::ClearFlag(uint32_t dwFlag) {
  m_uNodeFlags &= ~dwFlag;
}

bool CXFA_Node::IsAttributeInXML() {
  return JSNode()->GetEnum(XFA_Attribute::Contains) ==
         XFA_ATTRIBUTEENUM_MetaData;
}

void CXFA_Node::OnRemoved(bool bNotify) {
  if (!bNotify)
    return;

  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildRemoved();
}

void CXFA_Node::UpdateNameHash() {
  const XFA_NOTSUREATTRIBUTE* pNotsure =
      XFA_GetNotsureAttribute(GetElementType(), XFA_Attribute::Name);
  WideStringView wsName;
  if (!pNotsure || pNotsure->eType == XFA_AttributeType::CData) {
    wsName = JSNode()->GetCData(XFA_Attribute::Name);
    m_dwNameHash = FX_HashCode_GetW(wsName, false);
  } else if (pNotsure->eType == XFA_AttributeType::Enum) {
    wsName =
        GetAttributeEnumByID(JSNode()->GetEnum(XFA_Attribute::Name))->pName;
    m_dwNameHash = FX_HashCode_GetW(wsName, false);
  }
}

CFX_XMLNode* CXFA_Node::CreateXMLMappingNode() {
  if (!m_pXMLNode) {
    WideString wsTag(JSNode()->GetCData(XFA_Attribute::Name));
    m_pXMLNode = new CFX_XMLElement(wsTag);
    SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  return m_pXMLNode;
}

bool CXFA_Node::IsNeedSavingXMLNode() {
  return m_pXMLNode && (GetPacketID() == XFA_XDPPACKET_Datasets ||
                        GetElementType() == XFA_Element::Xfa);
}

CXFA_Node* CXFA_Node::GetItem(int32_t iIndex) {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideStringView wsName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
      WideStringView wsInstName = JSNode()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Right(wsInstName.GetLength() - 1) != wsName) {
        return nullptr;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
    if (iCount > iIndex)
      return pNode;
  }
  return nullptr;
}

int32_t CXFA_Node::GetCount() {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideStringView wsName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
      WideStringView wsInstName = JSNode()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Right(wsInstName.GetLength() - 1) != wsName) {
        return iCount;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
  }
  return iCount;
}

void CXFA_Node::InsertItem(CXFA_Node* pNewInstance,
                           int32_t iPos,
                           int32_t iCount,
                           bool bMoveDataBindingNodes) {
  if (iCount < 0)
    iCount = GetCount();
  if (iPos < 0)
    iPos = iCount;
  if (iPos == iCount) {
    CXFA_Node* pNextSibling =
        iCount > 0 ? GetItem(iCount - 1)->GetNodeItem(XFA_NODEITEM_NextSibling)
                   : GetNodeItem(XFA_NODEITEM_NextSibling);
    GetNodeItem(XFA_NODEITEM_Parent)->InsertChild(pNewInstance, pNextSibling);
    if (bMoveDataBindingNodes) {
      std::set<CXFA_Node*> sNew;
      std::set<CXFA_Node*> sAfter;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sNew.insert(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorAfter(pNextSibling);
      for (CXFA_Node* pNode = sIteratorAfter.GetCurrent(); pNode;
           pNode = sIteratorAfter.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sAfter.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sAfter, false);
    }
  } else {
    CXFA_Node* pBeforeInstance = GetItem(iPos);
    GetNodeItem(XFA_NODEITEM_Parent)
        ->InsertChild(pNewInstance, pBeforeInstance);
    if (bMoveDataBindingNodes) {
      std::set<CXFA_Node*> sNew;
      std::set<CXFA_Node*> sBefore;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sNew.insert(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorBefore(pBeforeInstance);
      for (CXFA_Node* pNode = sIteratorBefore.GetCurrent(); pNode;
           pNode = sIteratorBefore.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sBefore.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sBefore, true);
    }
  }
}

void CXFA_Node::RemoveItem(CXFA_Node* pRemoveInstance,
                           bool bRemoveDataBinding) {
  GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pRemoveInstance, true);
  if (!bRemoveDataBinding)
    return;

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
      sIterator(pRemoveInstance);
  for (CXFA_Node* pFormNode = sIterator.GetCurrent(); pFormNode;
       pFormNode = sIterator.MoveToNext()) {
    CXFA_Node* pDataNode = pFormNode->GetBindData();
    if (!pDataNode)
      continue;

    if (pDataNode->RemoveBindItem(pFormNode) == 0) {
      if (CXFA_Node* pDataParent =
              pDataNode->GetNodeItem(XFA_NODEITEM_Parent)) {
        pDataParent->RemoveChild(pDataNode, true);
      }
    }
    pFormNode->JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr,
                                   nullptr);
  }
}

CXFA_Node* CXFA_Node::CreateInstance(bool bDataMerge) {
  CXFA_Document* pDocument = GetDocument();
  CXFA_Node* pTemplateNode = GetTemplateNode();
  CXFA_Node* pFormParent = GetNodeItem(XFA_NODEITEM_Parent);
  CXFA_Node* pDataScope = nullptr;
  for (CXFA_Node* pRootBoundNode = pFormParent;
       pRootBoundNode && pRootBoundNode->IsContainerNode();
       pRootBoundNode = pRootBoundNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope)
      break;
  }
  if (!pDataScope) {
    pDataScope = ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Record));
    ASSERT(pDataScope);
  }
  CXFA_Node* pInstance = pDocument->DataMerge_CopyContainer(
      pTemplateNode, pFormParent, pDataScope, true, bDataMerge, true);
  if (pInstance) {
    pDocument->DataMerge_UpdateBindingRelations(pInstance);
    pFormParent->RemoveChild(pInstance, true);
  }
  return pInstance;
}


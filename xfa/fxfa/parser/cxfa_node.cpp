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
#include "xfa/fxfa/parser/cxfa_sharptext.h"
#include "xfa/fxfa/parser/cxfa_sharpxhtml.h"
#include "xfa/fxfa/parser/cxfa_sharpxml.h"
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

struct ElementNameInfo {
  uint32_t hash;
  XFA_Element element;
} ElementNameToEnum[] = {
    {0x23ee3, XFA_Element::Ps},
    {0x25363, XFA_Element::To},
    {0x2587e, XFA_Element::Ui},
    {0x1c648b, XFA_Element::RecordSet},
    {0x171428f, XFA_Element::SubsetBelow},
    {0x1a0776a, XFA_Element::SubformSet},
    {0x2340d70, XFA_Element::AdobeExtensionLevel},
    {0x2c1c7f1, XFA_Element::Typeface},
    {0x5518c25, XFA_Element::Break},
    {0x5fff523, XFA_Element::FontInfo},
    {0x653a227, XFA_Element::NumberPattern},
    {0x65b4a05, XFA_Element::DynamicRender},
    {0x7e4362e, XFA_Element::PrintScaling},
    {0x7fe6d3a, XFA_Element::CheckButton},
    {0x80cf58f, XFA_Element::DatePatterns},
    {0x811929d, XFA_Element::SourceSet},
    {0x9f9d612, XFA_Element::Amd},
    {0x9f9efb6, XFA_Element::Arc},
    {0xa48835e, XFA_Element::Day},
    {0xa6328b8, XFA_Element::Era},
    {0xae6a0a0, XFA_Element::Jog},
    {0xb1b3d22, XFA_Element::Log},
    {0xb35439e, XFA_Element::Map},
    {0xb355301, XFA_Element::Mdp},
    {0xb420438, XFA_Element::BreakBefore},
    {0xb6a091c, XFA_Element::Oid},
    {0xb84389f, XFA_Element::Pcl},
    {0xb843dba, XFA_Element::Pdf},
    {0xbb8df5d, XFA_Element::Ref},
    {0xc080cd0, XFA_Element::Uri},
    {0xc56afbf, XFA_Element::Xdc},
    {0xc56afcc, XFA_Element::Xdp},
    {0xc56b9ff, XFA_Element::Xfa},
    {0xc56fcb7, XFA_Element::Xsl},
    {0xc8b89d6, XFA_Element::Zpl},
    {0xc9bae94, XFA_Element::Cache},
    {0xcb016be, XFA_Element::Margin},
    {0xe1378fe, XFA_Element::KeyUsage},
    {0xfe3596a, XFA_Element::Exclude},
    {0x10395ac7, XFA_Element::ChoiceList},
    {0x1059ec18, XFA_Element::Level},
    {0x10874804, XFA_Element::LabelPrinter},
    {0x10c40e03, XFA_Element::CalendarSymbols},
    {0x10f1ea24, XFA_Element::Para},
    {0x10f1ea37, XFA_Element::Part},
    {0x1140975b, XFA_Element::Pdfa},
    {0x1154efe6, XFA_Element::Filter},
    {0x13f41de1, XFA_Element::Present},
    {0x1827e6ea, XFA_Element::Pagination},
    {0x18463707, XFA_Element::Encoding},
    {0x185e41e2, XFA_Element::Event},
    {0x1adb142d, XFA_Element::Whitespace},
    {0x1f3f64c3, XFA_Element::DefaultUi},
    {0x204e87cb, XFA_Element::DataModel},
    {0x2057b350, XFA_Element::Barcode},
    {0x20596bad, XFA_Element::TimePattern},
    {0x210b74d3, XFA_Element::BatchOutput},
    {0x212ff0e2, XFA_Element::Enforce},
    {0x21d351b4, XFA_Element::CurrencySymbols},
    {0x21db83c5, XFA_Element::AddSilentPrint},
    {0x22266258, XFA_Element::Rename},
    {0x226ca8f1, XFA_Element::Operation},
    {0x23e27b84, XFA_Element::Typefaces},
    {0x23f4aa75, XFA_Element::SubjectDNs},
    {0x240d5e8e, XFA_Element::Issuers},
    {0x2457a033, XFA_Element::SignaturePseudoModel},
    {0x24a52f8a, XFA_Element::WsdlConnection},
    {0x254ebd07, XFA_Element::Debug},
    {0x2655c66a, XFA_Element::Delta},
    {0x26c0daec, XFA_Element::EraNames},
    {0x273ab03b, XFA_Element::ModifyAnnots},
    {0x27875bb4, XFA_Element::StartNode},
    {0x285d0dbc, XFA_Element::Button},
    {0x28dee6e9, XFA_Element::Format},
    {0x2a23349e, XFA_Element::Border},
    {0x2ae67f19, XFA_Element::Area},
    {0x2c3c4c67, XFA_Element::Hyphenation},
    {0x2d08af85, XFA_Element::Text},
    {0x2d71b00f, XFA_Element::Time},
    {0x2f16a382, XFA_Element::Type},
    {0x2fe057e9, XFA_Element::Overprint},
    {0x302aee16, XFA_Element::Certificates},
    {0x30b227df, XFA_Element::EncryptionMethods},
    {0x32b900d1, XFA_Element::SetProperty},
    {0x337d9e45, XFA_Element::PrinterName},
    {0x33edda4b, XFA_Element::StartPage},
    {0x381943e4, XFA_Element::PageOffset},
    {0x382106cd, XFA_Element::DateTime},
    {0x386e7421, XFA_Element::Comb},
    {0x390acd9e, XFA_Element::Pattern},
    {0x3942163e, XFA_Element::IfEmpty},
    {0x39944a7b, XFA_Element::SuppressBanner},
    {0x3b3c3dca, XFA_Element::OutputBin},
    {0x3b8a4024, XFA_Element::Field},
    {0x3c15352f, XFA_Element::Agent},
    {0x3d7e8668, XFA_Element::OutputXSL},
    {0x3e1c91c5, XFA_Element::AdjustData},
    {0x3e7a9408, XFA_Element::AutoSave},
    {0x3ecead94, XFA_Element::ContentArea},
    {0x3ef334e3, XFA_Element::EventPseudoModel},
    {0x3fadaec0, XFA_Element::WsdlAddress},
    {0x40623b5b, XFA_Element::Solid},
    {0x41f0bd76, XFA_Element::DateTimeSymbols},
    {0x444e7523, XFA_Element::EncryptionLevel},
    {0x4523af55, XFA_Element::Edge},
    {0x45d5e3c1, XFA_Element::Stipple},
    {0x475e4e87, XFA_Element::Attributes},
    {0x487a8c87, XFA_Element::VersionControl},
    {0x48e5248c, XFA_Element::Meridiem},
    {0x48f36719, XFA_Element::ExclGroup},
    {0x4977356b, XFA_Element::ToolTip},
    {0x499afecc, XFA_Element::Compress},
    {0x4a0c4948, XFA_Element::Reason},
    {0x4bdcce13, XFA_Element::Execute},
    {0x4c56b216, XFA_Element::ContentCopy},
    {0x4cc176d3, XFA_Element::DateTimeEdit},
    {0x4e1e39b6, XFA_Element::Config},
    {0x4e2d6083, XFA_Element::Image},
    {0x4e814150, XFA_Element::SharpxHTML},
    {0x4f2388c1, XFA_Element::NumberOfCopies},
    {0x4f512e30, XFA_Element::BehaviorOverride},
    {0x4fdc3454, XFA_Element::TimeStamp},
    {0x51d90546, XFA_Element::Month},
    {0x523437e4, XFA_Element::ViewerPreferences},
    {0x53abc1c6, XFA_Element::ScriptModel},
    {0x54034c2f, XFA_Element::Decimal},
    {0x54202c9e, XFA_Element::Subform},
    {0x542c7300, XFA_Element::Select},
    {0x5436d198, XFA_Element::Window},
    {0x5473b6dc, XFA_Element::LocaleSet},
    {0x56ae179e, XFA_Element::Handler},
    {0x5700bd5f, XFA_Element::HostPseudoModel},
    {0x570ce835, XFA_Element::Presence},
    {0x5779d65f, XFA_Element::Record},
    {0x59c8f27d, XFA_Element::Embed},
    {0x5a50e9e6, XFA_Element::Version},
    {0x5b8383df, XFA_Element::Command},
    {0x5c43c6c3, XFA_Element::Copies},
    {0x5e0c2c49, XFA_Element::Staple},
    {0x5e5083dd, XFA_Element::SubmitFormat},
    {0x5e8c5d20, XFA_Element::Boolean},
    {0x60490a85, XFA_Element::Message},
    {0x60d4c8b1, XFA_Element::Output},
    {0x61810081, XFA_Element::PsMap},
    {0x62bd904b, XFA_Element::ExcludeNS},
    {0x669d4f77, XFA_Element::Assist},
    {0x67334a1c, XFA_Element::Picture},
    {0x67fe7334, XFA_Element::Traversal},
    {0x6894589c, XFA_Element::SilentPrint},
    {0x68a16bbd, XFA_Element::WebClient},
    {0x69f115df, XFA_Element::LayoutPseudoModel},
    {0x6a4bc084, XFA_Element::Producer},
    {0x6a9e04c9, XFA_Element::Corner},
    {0x6ccd7274, XFA_Element::MsgId},
    {0x6e67921f, XFA_Element::Color},
    {0x6ec217a5, XFA_Element::Keep},
    {0x6eef1116, XFA_Element::Query},
    {0x7033bfd5, XFA_Element::Insert},
    {0x704af389, XFA_Element::ImageEdit},
    {0x7233018a, XFA_Element::Validate},
    {0x72ba47b4, XFA_Element::DigestMethods},
    {0x72f2aa7a, XFA_Element::NumberPatterns},
    {0x74caed29, XFA_Element::PageSet},
    {0x7568e6ae, XFA_Element::Integer},
    {0x76182db9, XFA_Element::SoapAddress},
    {0x773146c5, XFA_Element::Equate},
    {0x77d449dd, XFA_Element::FormFieldFilling},
    {0x7889d68a, XFA_Element::PageRange},
    {0x7baca2e3, XFA_Element::Update},
    {0x7ce89001, XFA_Element::ConnectString},
    {0x7d9fd7c5, XFA_Element::Mode},
    {0x7e7e845e, XFA_Element::Layout},
    {0x7e845c34, XFA_Element::Sharpxml},
    {0x7fb341df, XFA_Element::XsdConnection},
    {0x7ffb51cc, XFA_Element::Traverse},
    {0x80203b5a, XFA_Element::Encodings},
    {0x803550fc, XFA_Element::Template},
    {0x803d5bbc, XFA_Element::Acrobat},
    {0x821d6569, XFA_Element::ValidationMessaging},
    {0x830e688f, XFA_Element::Signing},
    {0x83a550d2, XFA_Element::DataWindow},
    {0x83dab9f5, XFA_Element::Script},
    {0x8411ebcd, XFA_Element::AddViewerPreferences},
    {0x8777642e, XFA_Element::AlwaysEmbed},
    {0x877a6b39, XFA_Element::PasswordEdit},
    {0x87e84c99, XFA_Element::NumericEdit},
    {0x8852cdec, XFA_Element::EncryptionMethod},
    {0x891f4606, XFA_Element::Change},
    {0x89939f36, XFA_Element::PageArea},
    {0x8a9d6247, XFA_Element::SubmitUrl},
    {0x8ad8b90f, XFA_Element::Oids},
    {0x8b036f32, XFA_Element::Signature},
    {0x8b128efb, XFA_Element::ADBE_JSConsole},
    {0x8bcfe96e, XFA_Element::Caption},
    {0x8e1c2921, XFA_Element::Relevant},
    {0x8e3f0a4b, XFA_Element::FlipLabel},
    {0x900280b7, XFA_Element::ExData},
    {0x91e80352, XFA_Element::DayNames},
    {0x93113b11, XFA_Element::SoapAction},
    {0x938b09f6, XFA_Element::DefaultTypeface},
    {0x95b37897, XFA_Element::Manifest},
    {0x97b76b54, XFA_Element::Overflow},
    {0x9a57861b, XFA_Element::Linear},
    {0x9ad5a821, XFA_Element::CurrencySymbol},
    {0x9c6471b3, XFA_Element::Delete},
    {0x9deea61d, XFA_Element::Deltas},
    {0x9e67de21, XFA_Element::DigestMethod},
    {0x9f3e9510, XFA_Element::InstanceManager},
    {0xa0799892, XFA_Element::EquateRange},
    {0xa084a381, XFA_Element::Medium},
    {0xa1211b8b, XFA_Element::TextEdit},
    {0xa17008f0, XFA_Element::TemplateCache},
    {0xa4f7b88f, XFA_Element::CompressObjectStream},
    {0xa65f5d17, XFA_Element::DataValue},
    {0xa6caaa89, XFA_Element::AccessibleContent},
    {0xa94cc00b, XFA_Element::IncludeXDPContent},
    {0xa9b081a1, XFA_Element::XmlConnection},
    {0xab2a3b74, XFA_Element::ValidateApprovalSignatures},
    {0xab8c5a2b, XFA_Element::SignData},
    {0xabaa2ceb, XFA_Element::Packets},
    {0xadba359c, XFA_Element::DatePattern},
    {0xae222b2b, XFA_Element::DuplexOption},
    {0xb012effb, XFA_Element::Base},
    {0xb0e5485d, XFA_Element::Bind},
    {0xb45d61b2, XFA_Element::Compression},
    {0xb563f0ff, XFA_Element::User},
    {0xb5848ad5, XFA_Element::Rectangle},
    {0xb6dacb72, XFA_Element::EffectiveOutputPolicy},
    {0xb7d7654d, XFA_Element::ADBE_JSDebugger},
    {0xbab37f73, XFA_Element::Acrobat7},
    {0xbc70081e, XFA_Element::Interactive},
    {0xbc8fa350, XFA_Element::Locale},
    {0xbcd44940, XFA_Element::CurrentPage},
    {0xbde9abda, XFA_Element::Data},
    {0xbde9abde, XFA_Element::Date},
    {0xbe52dfbf, XFA_Element::Desc},
    {0xbf4b6405, XFA_Element::Encrypt},
    {0xbfa87cce, XFA_Element::Draw},
    {0xc181ff4b, XFA_Element::Encryption},
    {0xc1970f40, XFA_Element::MeridiemNames},
    {0xc5ad9f5e, XFA_Element::Messaging},
    {0xc69549f4, XFA_Element::Speak},
    {0xc7743dc7, XFA_Element::DataGroup},
    {0xc7eb20e9, XFA_Element::Common},
    {0xc85d4528, XFA_Element::Sharptext},
    {0xc861556a, XFA_Element::PaginationOverride},
    {0xc903dabb, XFA_Element::Reasons},
    {0xc9a8127f, XFA_Element::SignatureProperties},
    {0xca010c2d, XFA_Element::Threshold},
    {0xcc92aba7, XFA_Element::Fill},
    {0xcd308b77, XFA_Element::Font},
    {0xcd309ff4, XFA_Element::Form},
    {0xcebcca2d, XFA_Element::MediumInfo},
    {0xcfe0d643, XFA_Element::Certificate},
    {0xd012c033, XFA_Element::Password},
    {0xd01604bd, XFA_Element::RunScripts},
    {0xd1227e6f, XFA_Element::Trace},
    {0xd1532876, XFA_Element::Float},
    {0xd17a6c30, XFA_Element::RenderPolicy},
    {0xd4bf6823, XFA_Element::LogPseudoModel},
    {0xd58aa962, XFA_Element::Destination},
    {0xd6e27f1d, XFA_Element::Value},
    {0xd7a14462, XFA_Element::Bookend},
    {0xd8c31254, XFA_Element::ExObject},
    {0xda6a8590, XFA_Element::OpenAction},
    {0xdab4fb7d, XFA_Element::NeverEmbed},
    {0xdb98475f, XFA_Element::BindItems},
    {0xdbfbe02e, XFA_Element::Calculate},
    {0xdd7676ed, XFA_Element::Print},
    {0xdde273d7, XFA_Element::Extras},
    {0xde146b34, XFA_Element::Proto},
    {0xdf059321, XFA_Element::DSigData},
    {0xdfccf030, XFA_Element::Creator},
    {0xdff78c6a, XFA_Element::Connect},
    {0xe11a2cbc, XFA_Element::Permissions},
    {0xe14c801c, XFA_Element::ConnectionSet},
    {0xe1c83a14, XFA_Element::Submit},
    {0xe29821cd, XFA_Element::Range},
    {0xe38d83c7, XFA_Element::Linearized},
    {0xe3aa2578, XFA_Element::Packet},
    {0xe3aa860e, XFA_Element::RootElement},
    {0xe3e553fa, XFA_Element::PlaintextMetadata},
    {0xe3e6e4f2, XFA_Element::NumberSymbols},
    {0xe3f067f6, XFA_Element::PrintHighQuality},
    {0xe3fd078c, XFA_Element::Driver},
    {0xe48b34f2, XFA_Element::IncrementalLoad},
    {0xe550e7c2, XFA_Element::SubjectDN},
    {0xe6669a78, XFA_Element::CompressLogicalStructure},
    {0xe7a7ea02, XFA_Element::IncrementalMerge},
    {0xe948530d, XFA_Element::Radial},
    {0xea8d6999, XFA_Element::Variables},
    {0xeaa142c0, XFA_Element::TimePatterns},
    {0xeb943a71, XFA_Element::EffectiveInputPolicy},
    {0xef04a2bc, XFA_Element::NameAttr},
    {0xf07222ab, XFA_Element::Conformance},
    {0xf0aaaadc, XFA_Element::Transform},
    {0xf1433e88, XFA_Element::LockDocument},
    {0xf54eb997, XFA_Element::BreakAfter},
    {0xf616da28, XFA_Element::Line},
    {0xf616f3dc, XFA_Element::List},
    {0xf7055fb1, XFA_Element::Source},
    {0xf7eebe1c, XFA_Element::Occur},
    {0xf8d10d97, XFA_Element::PickTrayByPDFSize},
    {0xf8f19e3a, XFA_Element::MonthNames},
    {0xf984149b, XFA_Element::Severity},
    {0xf9bcb037, XFA_Element::GroupParent},
    {0xfbc42fff, XFA_Element::DocumentAssembly},
    {0xfc78159f, XFA_Element::NumberSymbol},
    {0xfcbd606c, XFA_Element::Tagged},
    {0xff063802, XFA_Element::Items},
};

}  // namespace

const XFA_ATTRIBUTEENUMINFO* GetAttributeEnumByID(XFA_ATTRIBUTEENUM eName) {
  return g_XFAEnumData + eName;
}

// static
XFA_Element CXFA_Node::NameToElement(const WideString& name) {
  uint32_t hash = FX_HashCode_GetW(name.AsStringView(), false);
  auto* elem = std::lower_bound(
      std::begin(ElementNameToEnum), std::end(ElementNameToEnum), hash,
      [](const ElementNameInfo& a, uint32_t hash) { return a.hash < hash; });
  if (elem && elem->hash == hash)
    return elem->element;
  return XFA_Element::Unknown;
}

// static
std::unique_ptr<CXFA_Node> CXFA_Node::Create(CXFA_Document* doc,
                                             XFA_Element element,
                                             XFA_XDPPACKET packet) {
  ASSERT(element != XFA_Element::NodeList);

  std::unique_ptr<CXFA_Node> node;
  switch (element) {
    case XFA_Element::Ps:
      node = pdfium::MakeUnique<CXFA_Ps>(doc, packet);
      break;
    case XFA_Element::To:
      node = pdfium::MakeUnique<CXFA_To>(doc, packet);
      break;
    case XFA_Element::Ui:
      node = pdfium::MakeUnique<CXFA_Ui>(doc, packet);
      break;
    case XFA_Element::RecordSet:
      node = pdfium::MakeUnique<CXFA_RecordSet>(doc, packet);
      break;
    case XFA_Element::SubsetBelow:
      node = pdfium::MakeUnique<CXFA_SubsetBelow>(doc, packet);
      break;
    case XFA_Element::SubformSet:
      node = pdfium::MakeUnique<CXFA_SubformSet>(doc, packet);
      break;
    case XFA_Element::AdobeExtensionLevel:
      node = pdfium::MakeUnique<CXFA_AdobeExtensionLevel>(doc, packet);
      break;
    case XFA_Element::Typeface:
      node = pdfium::MakeUnique<CXFA_Typeface>(doc, packet);
      break;
    case XFA_Element::Break:
      node = pdfium::MakeUnique<CXFA_Break>(doc, packet);
      break;
    case XFA_Element::FontInfo:
      node = pdfium::MakeUnique<CXFA_FontInfo>(doc, packet);
      break;
    case XFA_Element::NumberPattern:
      node = pdfium::MakeUnique<CXFA_NumberPattern>(doc, packet);
      break;
    case XFA_Element::DynamicRender:
      node = pdfium::MakeUnique<CXFA_DynamicRender>(doc, packet);
      break;
    case XFA_Element::PrintScaling:
      node = pdfium::MakeUnique<CXFA_PrintScaling>(doc, packet);
      break;
    case XFA_Element::CheckButton:
      node = pdfium::MakeUnique<CXFA_CheckButton>(doc, packet);
      break;
    case XFA_Element::DatePatterns:
      node = pdfium::MakeUnique<CXFA_DatePatterns>(doc, packet);
      break;
    case XFA_Element::SourceSet:
      node = pdfium::MakeUnique<CXFA_SourceSet>(doc, packet);
      break;
    case XFA_Element::Amd:
      node = pdfium::MakeUnique<CXFA_Amd>(doc, packet);
      break;
    case XFA_Element::Arc:
      node = pdfium::MakeUnique<CXFA_Arc>(doc, packet);
      break;
    case XFA_Element::Day:
      node = pdfium::MakeUnique<CXFA_Day>(doc, packet);
      break;
    case XFA_Element::Era:
      node = pdfium::MakeUnique<CXFA_Era>(doc, packet);
      break;
    case XFA_Element::Jog:
      node = pdfium::MakeUnique<CXFA_Jog>(doc, packet);
      break;
    case XFA_Element::Log:
      node = pdfium::MakeUnique<CXFA_Log>(doc, packet);
      break;
    case XFA_Element::Map:
      node = pdfium::MakeUnique<CXFA_Map>(doc, packet);
      break;
    case XFA_Element::Mdp:
      node = pdfium::MakeUnique<CXFA_Mdp>(doc, packet);
      break;
    case XFA_Element::BreakBefore:
      node = pdfium::MakeUnique<CXFA_BreakBefore>(doc, packet);
      break;
    case XFA_Element::Oid:
      node = pdfium::MakeUnique<CXFA_Oid>(doc, packet);
      break;
    case XFA_Element::Pcl:
      node = pdfium::MakeUnique<CXFA_Pcl>(doc, packet);
      break;
    case XFA_Element::Pdf:
      node = pdfium::MakeUnique<CXFA_Pdf>(doc, packet);
      break;
    case XFA_Element::Ref:
      node = pdfium::MakeUnique<CXFA_Ref>(doc, packet);
      break;
    case XFA_Element::Uri:
      node = pdfium::MakeUnique<CXFA_Uri>(doc, packet);
      break;
    case XFA_Element::Xdc:
      node = pdfium::MakeUnique<CXFA_Xdc>(doc, packet);
      break;
    case XFA_Element::Xdp:
      node = pdfium::MakeUnique<CXFA_Xdp>(doc, packet);
      break;
    case XFA_Element::Xfa:
      node = pdfium::MakeUnique<CXFA_Xfa>(doc, packet);
      break;
    case XFA_Element::Xsl:
      node = pdfium::MakeUnique<CXFA_Xsl>(doc, packet);
      break;
    case XFA_Element::Zpl:
      node = pdfium::MakeUnique<CXFA_Zpl>(doc, packet);
      break;
    case XFA_Element::Cache:
      node = pdfium::MakeUnique<CXFA_Cache>(doc, packet);
      break;
    case XFA_Element::Margin:
      node = pdfium::MakeUnique<CXFA_Margin>(doc, packet);
      break;
    case XFA_Element::KeyUsage:
      node = pdfium::MakeUnique<CXFA_KeyUsage>(doc, packet);
      break;
    case XFA_Element::Exclude:
      node = pdfium::MakeUnique<CXFA_Exclude>(doc, packet);
      break;
    case XFA_Element::ChoiceList:
      node = pdfium::MakeUnique<CXFA_ChoiceList>(doc, packet);
      break;
    case XFA_Element::Level:
      node = pdfium::MakeUnique<CXFA_Level>(doc, packet);
      break;
    case XFA_Element::LabelPrinter:
      node = pdfium::MakeUnique<CXFA_LabelPrinter>(doc, packet);
      break;
    case XFA_Element::CalendarSymbols:
      node = pdfium::MakeUnique<CXFA_CalendarSymbols>(doc, packet);
      break;
    case XFA_Element::Para:
      node = pdfium::MakeUnique<CXFA_Para>(doc, packet);
      break;
    case XFA_Element::Part:
      node = pdfium::MakeUnique<CXFA_Part>(doc, packet);
      break;
    case XFA_Element::Pdfa:
      node = pdfium::MakeUnique<CXFA_Pdfa>(doc, packet);
      break;
    case XFA_Element::Filter:
      node = pdfium::MakeUnique<CXFA_Filter>(doc, packet);
      break;
    case XFA_Element::Present:
      node = pdfium::MakeUnique<CXFA_Present>(doc, packet);
      break;
    case XFA_Element::Pagination:
      node = pdfium::MakeUnique<CXFA_Pagination>(doc, packet);
      break;
    case XFA_Element::Encoding:
      node = pdfium::MakeUnique<CXFA_Encoding>(doc, packet);
      break;
    case XFA_Element::Event:
      node = pdfium::MakeUnique<CXFA_Event>(doc, packet);
      break;
    case XFA_Element::Whitespace:
      node = pdfium::MakeUnique<CXFA_Whitespace>(doc, packet);
      break;
    case XFA_Element::DefaultUi:
      node = pdfium::MakeUnique<CXFA_DefaultUi>(doc, packet);
      break;
    case XFA_Element::DataModel:
      node = pdfium::MakeUnique<CXFA_DataModel>(doc, packet);
      break;
    case XFA_Element::Barcode:
      node = pdfium::MakeUnique<CXFA_Barcode>(doc, packet);
      break;
    case XFA_Element::TimePattern:
      node = pdfium::MakeUnique<CXFA_TimePattern>(doc, packet);
      break;
    case XFA_Element::BatchOutput:
      node = pdfium::MakeUnique<CXFA_BatchOutput>(doc, packet);
      break;
    case XFA_Element::Enforce:
      node = pdfium::MakeUnique<CXFA_Enforce>(doc, packet);
      break;
    case XFA_Element::CurrencySymbols:
      node = pdfium::MakeUnique<CXFA_CurrencySymbols>(doc, packet);
      break;
    case XFA_Element::AddSilentPrint:
      node = pdfium::MakeUnique<CXFA_AddSilentPrint>(doc, packet);
      break;
    case XFA_Element::Rename:
      node = pdfium::MakeUnique<CXFA_Rename>(doc, packet);
      break;
    case XFA_Element::Operation:
      node = pdfium::MakeUnique<CXFA_Operation>(doc, packet);
      break;
    case XFA_Element::Typefaces:
      node = pdfium::MakeUnique<CXFA_Typefaces>(doc, packet);
      break;
    case XFA_Element::SubjectDNs:
      node = pdfium::MakeUnique<CXFA_SubjectDNs>(doc, packet);
      break;
    case XFA_Element::Issuers:
      node = pdfium::MakeUnique<CXFA_Issuers>(doc, packet);
      break;
    case XFA_Element::SignaturePseudoModel:
      node = pdfium::MakeUnique<CXFA_SignaturePseudoModel>(doc, packet);
      break;
    case XFA_Element::WsdlConnection:
      node = pdfium::MakeUnique<CXFA_WsdlConnection>(doc, packet);
      break;
    case XFA_Element::Debug:
      node = pdfium::MakeUnique<CXFA_Debug>(doc, packet);
      break;
    case XFA_Element::Delta:
      node = pdfium::MakeUnique<CXFA_Delta>(doc, packet);
      break;
    case XFA_Element::EraNames:
      node = pdfium::MakeUnique<CXFA_EraNames>(doc, packet);
      break;
    case XFA_Element::ModifyAnnots:
      node = pdfium::MakeUnique<CXFA_ModifyAnnots>(doc, packet);
      break;
    case XFA_Element::StartNode:
      node = pdfium::MakeUnique<CXFA_StartNode>(doc, packet);
      break;
    case XFA_Element::Button:
      node = pdfium::MakeUnique<CXFA_Button>(doc, packet);
      break;
    case XFA_Element::Format:
      node = pdfium::MakeUnique<CXFA_Format>(doc, packet);
      break;
    case XFA_Element::Border:
      node = pdfium::MakeUnique<CXFA_Border>(doc, packet);
      break;
    case XFA_Element::Area:
      node = pdfium::MakeUnique<CXFA_Area>(doc, packet);
      break;
    case XFA_Element::Hyphenation:
      node = pdfium::MakeUnique<CXFA_Hyphenation>(doc, packet);
      break;
    case XFA_Element::Text:
      node = pdfium::MakeUnique<CXFA_Text>(doc, packet);
      break;
    case XFA_Element::Time:
      node = pdfium::MakeUnique<CXFA_Time>(doc, packet);
      break;
    case XFA_Element::Type:
      node = pdfium::MakeUnique<CXFA_Type>(doc, packet);
      break;
    case XFA_Element::Overprint:
      node = pdfium::MakeUnique<CXFA_Overprint>(doc, packet);
      break;
    case XFA_Element::Certificates:
      node = pdfium::MakeUnique<CXFA_Certificates>(doc, packet);
      break;
    case XFA_Element::EncryptionMethods:
      node = pdfium::MakeUnique<CXFA_EncryptionMethods>(doc, packet);
      break;
    case XFA_Element::SetProperty:
      node = pdfium::MakeUnique<CXFA_SetProperty>(doc, packet);
      break;
    case XFA_Element::PrinterName:
      node = pdfium::MakeUnique<CXFA_PrinterName>(doc, packet);
      break;
    case XFA_Element::StartPage:
      node = pdfium::MakeUnique<CXFA_StartPage>(doc, packet);
      break;
    case XFA_Element::PageOffset:
      node = pdfium::MakeUnique<CXFA_PageOffset>(doc, packet);
      break;
    case XFA_Element::DateTime:
      node = pdfium::MakeUnique<CXFA_DateTime>(doc, packet);
      break;
    case XFA_Element::Comb:
      node = pdfium::MakeUnique<CXFA_Comb>(doc, packet);
      break;
    case XFA_Element::Pattern:
      node = pdfium::MakeUnique<CXFA_Pattern>(doc, packet);
      break;
    case XFA_Element::IfEmpty:
      node = pdfium::MakeUnique<CXFA_IfEmpty>(doc, packet);
      break;
    case XFA_Element::SuppressBanner:
      node = pdfium::MakeUnique<CXFA_SuppressBanner>(doc, packet);
      break;
    case XFA_Element::OutputBin:
      node = pdfium::MakeUnique<CXFA_OutputBin>(doc, packet);
      break;
    case XFA_Element::Field:
      node = pdfium::MakeUnique<CXFA_Field>(doc, packet);
      break;
    case XFA_Element::Agent:
      node = pdfium::MakeUnique<CXFA_Agent>(doc, packet);
      break;
    case XFA_Element::OutputXSL:
      node = pdfium::MakeUnique<CXFA_OutputXSL>(doc, packet);
      break;
    case XFA_Element::AdjustData:
      node = pdfium::MakeUnique<CXFA_AdjustData>(doc, packet);
      break;
    case XFA_Element::AutoSave:
      node = pdfium::MakeUnique<CXFA_AutoSave>(doc, packet);
      break;
    case XFA_Element::ContentArea:
      node = pdfium::MakeUnique<CXFA_ContentArea>(doc, packet);
      break;
    case XFA_Element::EventPseudoModel:
      node = pdfium::MakeUnique<CXFA_EventPseudoModel>(doc, packet);
      break;
    case XFA_Element::WsdlAddress:
      node = pdfium::MakeUnique<CXFA_WsdlAddress>(doc, packet);
      break;
    case XFA_Element::Solid:
      node = pdfium::MakeUnique<CXFA_Solid>(doc, packet);
      break;
    case XFA_Element::DateTimeSymbols:
      node = pdfium::MakeUnique<CXFA_DateTimeSymbols>(doc, packet);
      break;
    case XFA_Element::EncryptionLevel:
      node = pdfium::MakeUnique<CXFA_EncryptionLevel>(doc, packet);
      break;
    case XFA_Element::Edge:
      node = pdfium::MakeUnique<CXFA_Edge>(doc, packet);
      break;
    case XFA_Element::Stipple:
      node = pdfium::MakeUnique<CXFA_Stipple>(doc, packet);
      break;
    case XFA_Element::Attributes:
      node = pdfium::MakeUnique<CXFA_Attributes>(doc, packet);
      break;
    case XFA_Element::VersionControl:
      node = pdfium::MakeUnique<CXFA_VersionControl>(doc, packet);
      break;
    case XFA_Element::Meridiem:
      node = pdfium::MakeUnique<CXFA_Meridiem>(doc, packet);
      break;
    case XFA_Element::ExclGroup:
      node = pdfium::MakeUnique<CXFA_ExclGroup>(doc, packet);
      break;
    case XFA_Element::ToolTip:
      node = pdfium::MakeUnique<CXFA_ToolTip>(doc, packet);
      break;
    case XFA_Element::Compress:
      node = pdfium::MakeUnique<CXFA_Compress>(doc, packet);
      break;
    case XFA_Element::Reason:
      node = pdfium::MakeUnique<CXFA_Reason>(doc, packet);
      break;
    case XFA_Element::Execute:
      node = pdfium::MakeUnique<CXFA_Execute>(doc, packet);
      break;
    case XFA_Element::ContentCopy:
      node = pdfium::MakeUnique<CXFA_ContentCopy>(doc, packet);
      break;
    case XFA_Element::DateTimeEdit:
      node = pdfium::MakeUnique<CXFA_DateTimeEdit>(doc, packet);
      break;
    case XFA_Element::Config:
      node = pdfium::MakeUnique<CXFA_Config>(doc, packet);
      break;
    case XFA_Element::Image:
      node = pdfium::MakeUnique<CXFA_Image>(doc, packet);
      break;
    case XFA_Element::SharpxHTML:
      node = pdfium::MakeUnique<CXFA_SharpxHTML>(doc, packet);
      break;
    case XFA_Element::NumberOfCopies:
      node = pdfium::MakeUnique<CXFA_NumberOfCopies>(doc, packet);
      break;
    case XFA_Element::BehaviorOverride:
      node = pdfium::MakeUnique<CXFA_BehaviorOverride>(doc, packet);
      break;
    case XFA_Element::TimeStamp:
      node = pdfium::MakeUnique<CXFA_TimeStamp>(doc, packet);
      break;
    case XFA_Element::Month:
      node = pdfium::MakeUnique<CXFA_Month>(doc, packet);
      break;
    case XFA_Element::ViewerPreferences:
      node = pdfium::MakeUnique<CXFA_ViewerPreferences>(doc, packet);
      break;
    case XFA_Element::ScriptModel:
      node = pdfium::MakeUnique<CXFA_ScriptModel>(doc, packet);
      break;
    case XFA_Element::Decimal:
      node = pdfium::MakeUnique<CXFA_Decimal>(doc, packet);
      break;
    case XFA_Element::Subform:
      node = pdfium::MakeUnique<CXFA_Subform>(doc, packet);
      break;
    case XFA_Element::Select:
      node = pdfium::MakeUnique<CXFA_Select>(doc, packet);
      break;
    case XFA_Element::Window:
      node = pdfium::MakeUnique<CXFA_Window>(doc, packet);
      break;
    case XFA_Element::LocaleSet:
      node = pdfium::MakeUnique<CXFA_LocaleSet>(doc, packet);
      break;
    case XFA_Element::Handler:
      node = pdfium::MakeUnique<CXFA_Handler>(doc, packet);
      break;
    case XFA_Element::HostPseudoModel:
      node = pdfium::MakeUnique<CXFA_HostPseudoModel>(doc, packet);
      break;
    case XFA_Element::Presence:
      node = pdfium::MakeUnique<CXFA_Presence>(doc, packet);
      break;
    case XFA_Element::Record:
      node = pdfium::MakeUnique<CXFA_Record>(doc, packet);
      break;
    case XFA_Element::Embed:
      node = pdfium::MakeUnique<CXFA_Embed>(doc, packet);
      break;
    case XFA_Element::Version:
      node = pdfium::MakeUnique<CXFA_Version>(doc, packet);
      break;
    case XFA_Element::Command:
      node = pdfium::MakeUnique<CXFA_Command>(doc, packet);
      break;
    case XFA_Element::Copies:
      node = pdfium::MakeUnique<CXFA_Copies>(doc, packet);
      break;
    case XFA_Element::Staple:
      node = pdfium::MakeUnique<CXFA_Staple>(doc, packet);
      break;
    case XFA_Element::SubmitFormat:
      node = pdfium::MakeUnique<CXFA_SubmitFormat>(doc, packet);
      break;
    case XFA_Element::Boolean:
      node = pdfium::MakeUnique<CXFA_Boolean>(doc, packet);
      break;
    case XFA_Element::Message:
      node = pdfium::MakeUnique<CXFA_Message>(doc, packet);
      break;
    case XFA_Element::Output:
      node = pdfium::MakeUnique<CXFA_Output>(doc, packet);
      break;
    case XFA_Element::PsMap:
      node = pdfium::MakeUnique<CXFA_PsMap>(doc, packet);
      break;
    case XFA_Element::ExcludeNS:
      node = pdfium::MakeUnique<CXFA_ExcludeNS>(doc, packet);
      break;
    case XFA_Element::Assist:
      node = pdfium::MakeUnique<CXFA_Assist>(doc, packet);
      break;
    case XFA_Element::Picture:
      node = pdfium::MakeUnique<CXFA_Picture>(doc, packet);
      break;
    case XFA_Element::Traversal:
      node = pdfium::MakeUnique<CXFA_Traversal>(doc, packet);
      break;
    case XFA_Element::SilentPrint:
      node = pdfium::MakeUnique<CXFA_SilentPrint>(doc, packet);
      break;
    case XFA_Element::WebClient:
      node = pdfium::MakeUnique<CXFA_WebClient>(doc, packet);
      break;
    case XFA_Element::LayoutPseudoModel:
      node = pdfium::MakeUnique<CXFA_LayoutPseudoModel>(doc, packet);
      break;
    case XFA_Element::Producer:
      node = pdfium::MakeUnique<CXFA_Producer>(doc, packet);
      break;
    case XFA_Element::Corner:
      node = pdfium::MakeUnique<CXFA_Corner>(doc, packet);
      break;
    case XFA_Element::MsgId:
      node = pdfium::MakeUnique<CXFA_MsgId>(doc, packet);
      break;
    case XFA_Element::Color:
      node = pdfium::MakeUnique<CXFA_Color>(doc, packet);
      break;
    case XFA_Element::Keep:
      node = pdfium::MakeUnique<CXFA_Keep>(doc, packet);
      break;
    case XFA_Element::Query:
      node = pdfium::MakeUnique<CXFA_Query>(doc, packet);
      break;
    case XFA_Element::Insert:
      node = pdfium::MakeUnique<CXFA_Insert>(doc, packet);
      break;
    case XFA_Element::ImageEdit:
      node = pdfium::MakeUnique<CXFA_ImageEdit>(doc, packet);
      break;
    case XFA_Element::Validate:
      node = pdfium::MakeUnique<CXFA_Validate>(doc, packet);
      break;
    case XFA_Element::DigestMethods:
      node = pdfium::MakeUnique<CXFA_DigestMethods>(doc, packet);
      break;
    case XFA_Element::NumberPatterns:
      node = pdfium::MakeUnique<CXFA_NumberPatterns>(doc, packet);
      break;
    case XFA_Element::PageSet:
      node = pdfium::MakeUnique<CXFA_PageSet>(doc, packet);
      break;
    case XFA_Element::Integer:
      node = pdfium::MakeUnique<CXFA_Integer>(doc, packet);
      break;
    case XFA_Element::SoapAddress:
      node = pdfium::MakeUnique<CXFA_SoapAddress>(doc, packet);
      break;
    case XFA_Element::Equate:
      node = pdfium::MakeUnique<CXFA_Equate>(doc, packet);
      break;
    case XFA_Element::FormFieldFilling:
      node = pdfium::MakeUnique<CXFA_FormFieldFilling>(doc, packet);
      break;
    case XFA_Element::PageRange:
      node = pdfium::MakeUnique<CXFA_PageRange>(doc, packet);
      break;
    case XFA_Element::Update:
      node = pdfium::MakeUnique<CXFA_Update>(doc, packet);
      break;
    case XFA_Element::ConnectString:
      node = pdfium::MakeUnique<CXFA_ConnectString>(doc, packet);
      break;
    case XFA_Element::Mode:
      node = pdfium::MakeUnique<CXFA_Mode>(doc, packet);
      break;
    case XFA_Element::Layout:
      node = pdfium::MakeUnique<CXFA_Layout>(doc, packet);
      break;
    case XFA_Element::Sharpxml:
      node = pdfium::MakeUnique<CXFA_Sharpxml>(doc, packet);
      break;
    case XFA_Element::XsdConnection:
      node = pdfium::MakeUnique<CXFA_XsdConnection>(doc, packet);
      break;
    case XFA_Element::Traverse:
      node = pdfium::MakeUnique<CXFA_Traverse>(doc, packet);
      break;
    case XFA_Element::Encodings:
      node = pdfium::MakeUnique<CXFA_Encodings>(doc, packet);
      break;
    case XFA_Element::Template:
      node = pdfium::MakeUnique<CXFA_Template>(doc, packet);
      break;
    case XFA_Element::Acrobat:
      node = pdfium::MakeUnique<CXFA_Acrobat>(doc, packet);
      break;
    case XFA_Element::ValidationMessaging:
      node = pdfium::MakeUnique<CXFA_ValidationMessaging>(doc, packet);
      break;
    case XFA_Element::Signing:
      node = pdfium::MakeUnique<CXFA_Signing>(doc, packet);
      break;
    case XFA_Element::DataWindow:
      node = pdfium::MakeUnique<CXFA_DataWindow>(doc, packet);
      break;
    case XFA_Element::Script:
      node = pdfium::MakeUnique<CXFA_Script>(doc, packet);
      break;
    case XFA_Element::AddViewerPreferences:
      node = pdfium::MakeUnique<CXFA_AddViewerPreferences>(doc, packet);
      break;
    case XFA_Element::AlwaysEmbed:
      node = pdfium::MakeUnique<CXFA_AlwaysEmbed>(doc, packet);
      break;
    case XFA_Element::PasswordEdit:
      node = pdfium::MakeUnique<CXFA_PasswordEdit>(doc, packet);
      break;
    case XFA_Element::NumericEdit:
      node = pdfium::MakeUnique<CXFA_NumericEdit>(doc, packet);
      break;
    case XFA_Element::EncryptionMethod:
      node = pdfium::MakeUnique<CXFA_EncryptionMethod>(doc, packet);
      break;
    case XFA_Element::Change:
      node = pdfium::MakeUnique<CXFA_Change>(doc, packet);
      break;
    case XFA_Element::PageArea:
      node = pdfium::MakeUnique<CXFA_PageArea>(doc, packet);
      break;
    case XFA_Element::SubmitUrl:
      node = pdfium::MakeUnique<CXFA_SubmitUrl>(doc, packet);
      break;
    case XFA_Element::Oids:
      node = pdfium::MakeUnique<CXFA_Oids>(doc, packet);
      break;
    case XFA_Element::Signature:
      node = pdfium::MakeUnique<CXFA_Signature>(doc, packet);
      break;
    case XFA_Element::ADBE_JSConsole:
      node = pdfium::MakeUnique<CXFA_aDBE_JSConsole>(doc, packet);
      break;
    case XFA_Element::Caption:
      node = pdfium::MakeUnique<CXFA_Caption>(doc, packet);
      break;
    case XFA_Element::Relevant:
      node = pdfium::MakeUnique<CXFA_Relevant>(doc, packet);
      break;
    case XFA_Element::FlipLabel:
      node = pdfium::MakeUnique<CXFA_FlipLabel>(doc, packet);
      break;
    case XFA_Element::ExData:
      node = pdfium::MakeUnique<CXFA_ExData>(doc, packet);
      break;
    case XFA_Element::DayNames:
      node = pdfium::MakeUnique<CXFA_DayNames>(doc, packet);
      break;
    case XFA_Element::SoapAction:
      node = pdfium::MakeUnique<CXFA_SoapAction>(doc, packet);
      break;
    case XFA_Element::DefaultTypeface:
      node = pdfium::MakeUnique<CXFA_DefaultTypeface>(doc, packet);
      break;
    case XFA_Element::Manifest:
      node = pdfium::MakeUnique<CXFA_Manifest>(doc, packet);
      break;
    case XFA_Element::Overflow:
      node = pdfium::MakeUnique<CXFA_Overflow>(doc, packet);
      break;
    case XFA_Element::Linear:
      node = pdfium::MakeUnique<CXFA_Linear>(doc, packet);
      break;
    case XFA_Element::CurrencySymbol:
      node = pdfium::MakeUnique<CXFA_CurrencySymbol>(doc, packet);
      break;
    case XFA_Element::Delete:
      node = pdfium::MakeUnique<CXFA_Delete>(doc, packet);
      break;
    case XFA_Element::Deltas:
      node = pdfium::MakeUnique<CXFA_Deltas>(doc, packet);
      break;
    case XFA_Element::DigestMethod:
      node = pdfium::MakeUnique<CXFA_DigestMethod>(doc, packet);
      break;
    case XFA_Element::InstanceManager:
      node = pdfium::MakeUnique<CXFA_InstanceManager>(doc, packet);
      break;
    case XFA_Element::EquateRange:
      node = pdfium::MakeUnique<CXFA_EquateRange>(doc, packet);
      break;
    case XFA_Element::Medium:
      node = pdfium::MakeUnique<CXFA_Medium>(doc, packet);
      break;
    case XFA_Element::TextEdit:
      node = pdfium::MakeUnique<CXFA_TextEdit>(doc, packet);
      break;
    case XFA_Element::TemplateCache:
      node = pdfium::MakeUnique<CXFA_TemplateCache>(doc, packet);
      break;
    case XFA_Element::CompressObjectStream:
      node = pdfium::MakeUnique<CXFA_CompressObjectStream>(doc, packet);
      break;
    case XFA_Element::DataValue:
      node = pdfium::MakeUnique<CXFA_DataValue>(doc, packet);
      break;
    case XFA_Element::AccessibleContent:
      node = pdfium::MakeUnique<CXFA_AccessibleContent>(doc, packet);
      break;
    case XFA_Element::IncludeXDPContent:
      node = pdfium::MakeUnique<CXFA_IncludeXDPContent>(doc, packet);
      break;
    case XFA_Element::XmlConnection:
      node = pdfium::MakeUnique<CXFA_XmlConnection>(doc, packet);
      break;
    case XFA_Element::ValidateApprovalSignatures:
      node = pdfium::MakeUnique<CXFA_ValidateApprovalSignatures>(doc, packet);
      break;
    case XFA_Element::SignData:
      node = pdfium::MakeUnique<CXFA_SignData>(doc, packet);
      break;
    case XFA_Element::Packets:
      node = pdfium::MakeUnique<CXFA_Packets>(doc, packet);
      break;
    case XFA_Element::DatePattern:
      node = pdfium::MakeUnique<CXFA_DatePattern>(doc, packet);
      break;
    case XFA_Element::DuplexOption:
      node = pdfium::MakeUnique<CXFA_DuplexOption>(doc, packet);
      break;
    case XFA_Element::Base:
      node = pdfium::MakeUnique<CXFA_Base>(doc, packet);
      break;
    case XFA_Element::Bind:
      node = pdfium::MakeUnique<CXFA_Bind>(doc, packet);
      break;
    case XFA_Element::Compression:
      node = pdfium::MakeUnique<CXFA_Compression>(doc, packet);
      break;
    case XFA_Element::User:
      node = pdfium::MakeUnique<CXFA_User>(doc, packet);
      break;
    case XFA_Element::Rectangle:
      node = pdfium::MakeUnique<CXFA_Rectangle>(doc, packet);
      break;
    case XFA_Element::EffectiveOutputPolicy:
      node = pdfium::MakeUnique<CXFA_EffectiveOutputPolicy>(doc, packet);
      break;
    case XFA_Element::ADBE_JSDebugger:
      node = pdfium::MakeUnique<CXFA_aDBE_JSDebugger>(doc, packet);
      break;
    case XFA_Element::Acrobat7:
      node = pdfium::MakeUnique<CXFA_Acrobat7>(doc, packet);
      break;
    case XFA_Element::Interactive:
      node = pdfium::MakeUnique<CXFA_Interactive>(doc, packet);
      break;
    case XFA_Element::Locale:
      node = pdfium::MakeUnique<CXFA_Locale>(doc, packet);
      break;
    case XFA_Element::CurrentPage:
      node = pdfium::MakeUnique<CXFA_CurrentPage>(doc, packet);
      break;
    case XFA_Element::Data:
      node = pdfium::MakeUnique<CXFA_Data>(doc, packet);
      break;
    case XFA_Element::Date:
      node = pdfium::MakeUnique<CXFA_Date>(doc, packet);
      break;
    case XFA_Element::Desc:
      node = pdfium::MakeUnique<CXFA_Desc>(doc, packet);
      break;
    case XFA_Element::Encrypt:
      node = pdfium::MakeUnique<CXFA_Encrypt>(doc, packet);
      break;
    case XFA_Element::Draw:
      node = pdfium::MakeUnique<CXFA_Draw>(doc, packet);
      break;
    case XFA_Element::Encryption:
      node = pdfium::MakeUnique<CXFA_Encryption>(doc, packet);
      break;
    case XFA_Element::MeridiemNames:
      node = pdfium::MakeUnique<CXFA_MeridiemNames>(doc, packet);
      break;
    case XFA_Element::Messaging:
      node = pdfium::MakeUnique<CXFA_Messaging>(doc, packet);
      break;
    case XFA_Element::Speak:
      node = pdfium::MakeUnique<CXFA_Speak>(doc, packet);
      break;
    case XFA_Element::DataGroup:
      node = pdfium::MakeUnique<CXFA_DataGroup>(doc, packet);
      break;
    case XFA_Element::Common:
      node = pdfium::MakeUnique<CXFA_Common>(doc, packet);
      break;
    case XFA_Element::Sharptext:
      node = pdfium::MakeUnique<CXFA_Sharptext>(doc, packet);
      break;
    case XFA_Element::PaginationOverride:
      node = pdfium::MakeUnique<CXFA_PaginationOverride>(doc, packet);
      break;
    case XFA_Element::Reasons:
      node = pdfium::MakeUnique<CXFA_Reasons>(doc, packet);
      break;
    case XFA_Element::SignatureProperties:
      node = pdfium::MakeUnique<CXFA_SignatureProperties>(doc, packet);
      break;
    case XFA_Element::Threshold:
      node = pdfium::MakeUnique<CXFA_Threshold>(doc, packet);
      break;
    case XFA_Element::AppearanceFilter:
      node = pdfium::MakeUnique<CXFA_AppearanceFilter>(doc, packet);
      break;
    case XFA_Element::Fill:
      node = pdfium::MakeUnique<CXFA_Fill>(doc, packet);
      break;
    case XFA_Element::Font:
      node = pdfium::MakeUnique<CXFA_Font>(doc, packet);
      break;
    case XFA_Element::Form:
      node = pdfium::MakeUnique<CXFA_Form>(doc, packet);
      break;
    case XFA_Element::MediumInfo:
      node = pdfium::MakeUnique<CXFA_MediumInfo>(doc, packet);
      break;
    case XFA_Element::Certificate:
      node = pdfium::MakeUnique<CXFA_Certificate>(doc, packet);
      break;
    case XFA_Element::Password:
      node = pdfium::MakeUnique<CXFA_Password>(doc, packet);
      break;
    case XFA_Element::RunScripts:
      node = pdfium::MakeUnique<CXFA_RunScripts>(doc, packet);
      break;
    case XFA_Element::Trace:
      node = pdfium::MakeUnique<CXFA_Trace>(doc, packet);
      break;
    case XFA_Element::Float:
      node = pdfium::MakeUnique<CXFA_Float>(doc, packet);
      break;
    case XFA_Element::RenderPolicy:
      node = pdfium::MakeUnique<CXFA_RenderPolicy>(doc, packet);
      break;
    case XFA_Element::LogPseudoModel:
      node = pdfium::MakeUnique<CXFA_LogPseudoModel>(doc, packet);
      break;
    case XFA_Element::Destination:
      node = pdfium::MakeUnique<CXFA_Destination>(doc, packet);
      break;
    case XFA_Element::Value:
      node = pdfium::MakeUnique<CXFA_Value>(doc, packet);
      break;
    case XFA_Element::Bookend:
      node = pdfium::MakeUnique<CXFA_Bookend>(doc, packet);
      break;
    case XFA_Element::ExObject:
      node = pdfium::MakeUnique<CXFA_ExObject>(doc, packet);
      break;
    case XFA_Element::OpenAction:
      node = pdfium::MakeUnique<CXFA_OpenAction>(doc, packet);
      break;
    case XFA_Element::NeverEmbed:
      node = pdfium::MakeUnique<CXFA_NeverEmbed>(doc, packet);
      break;
    case XFA_Element::BindItems:
      node = pdfium::MakeUnique<CXFA_BindItems>(doc, packet);
      break;
    case XFA_Element::Calculate:
      node = pdfium::MakeUnique<CXFA_Calculate>(doc, packet);
      break;
    case XFA_Element::Print:
      node = pdfium::MakeUnique<CXFA_Print>(doc, packet);
      break;
    case XFA_Element::Extras:
      node = pdfium::MakeUnique<CXFA_Extras>(doc, packet);
      break;
    case XFA_Element::Proto:
      node = pdfium::MakeUnique<CXFA_Proto>(doc, packet);
      break;
    case XFA_Element::DSigData:
      node = pdfium::MakeUnique<CXFA_DSigData>(doc, packet);
      break;
    case XFA_Element::Creator:
      node = pdfium::MakeUnique<CXFA_Creator>(doc, packet);
      break;
    case XFA_Element::Connect:
      node = pdfium::MakeUnique<CXFA_Connect>(doc, packet);
      break;
    case XFA_Element::Permissions:
      node = pdfium::MakeUnique<CXFA_Permissions>(doc, packet);
      break;
    case XFA_Element::ConnectionSet:
      node = pdfium::MakeUnique<CXFA_ConnectionSet>(doc, packet);
      break;
    case XFA_Element::Submit:
      node = pdfium::MakeUnique<CXFA_Submit>(doc, packet);
      break;
    case XFA_Element::Range:
      node = pdfium::MakeUnique<CXFA_Range>(doc, packet);
      break;
    case XFA_Element::Linearized:
      node = pdfium::MakeUnique<CXFA_Linearized>(doc, packet);
      break;
    case XFA_Element::Packet:
      node = pdfium::MakeUnique<CXFA_Packet>(doc, packet);
      break;
    case XFA_Element::RootElement:
      node = pdfium::MakeUnique<CXFA_RootElement>(doc, packet);
      break;
    case XFA_Element::PlaintextMetadata:
      node = pdfium::MakeUnique<CXFA_PlaintextMetadata>(doc, packet);
      break;
    case XFA_Element::NumberSymbols:
      node = pdfium::MakeUnique<CXFA_NumberSymbols>(doc, packet);
      break;
    case XFA_Element::PrintHighQuality:
      node = pdfium::MakeUnique<CXFA_PrintHighQuality>(doc, packet);
      break;
    case XFA_Element::Driver:
      node = pdfium::MakeUnique<CXFA_Driver>(doc, packet);
      break;
    case XFA_Element::IncrementalLoad:
      node = pdfium::MakeUnique<CXFA_IncrementalLoad>(doc, packet);
      break;
    case XFA_Element::SubjectDN:
      node = pdfium::MakeUnique<CXFA_SubjectDN>(doc, packet);
      break;
    case XFA_Element::CompressLogicalStructure:
      node = pdfium::MakeUnique<CXFA_CompressLogicalStructure>(doc, packet);
      break;
    case XFA_Element::IncrementalMerge:
      node = pdfium::MakeUnique<CXFA_IncrementalMerge>(doc, packet);
      break;
    case XFA_Element::Radial:
      node = pdfium::MakeUnique<CXFA_Radial>(doc, packet);
      break;
    case XFA_Element::Variables:
      node = pdfium::MakeUnique<CXFA_Variables>(doc, packet);
      break;
    case XFA_Element::TimePatterns:
      node = pdfium::MakeUnique<CXFA_TimePatterns>(doc, packet);
      break;
    case XFA_Element::EffectiveInputPolicy:
      node = pdfium::MakeUnique<CXFA_EffectiveInputPolicy>(doc, packet);
      break;
    case XFA_Element::NameAttr:
      node = pdfium::MakeUnique<CXFA_NameAttr>(doc, packet);
      break;
    case XFA_Element::Conformance:
      node = pdfium::MakeUnique<CXFA_Conformance>(doc, packet);
      break;
    case XFA_Element::Transform:
      node = pdfium::MakeUnique<CXFA_Transform>(doc, packet);
      break;
    case XFA_Element::LockDocument:
      node = pdfium::MakeUnique<CXFA_LockDocument>(doc, packet);
      break;
    case XFA_Element::BreakAfter:
      node = pdfium::MakeUnique<CXFA_BreakAfter>(doc, packet);
      break;
    case XFA_Element::Line:
      node = pdfium::MakeUnique<CXFA_Line>(doc, packet);
      break;
    case XFA_Element::List:
      node = pdfium::MakeUnique<CXFA_List>(doc, packet);
      break;
    case XFA_Element::Source:
      node = pdfium::MakeUnique<CXFA_Source>(doc, packet);
      break;
    case XFA_Element::Occur:
      node = pdfium::MakeUnique<CXFA_Occur>(doc, packet);
      break;
    case XFA_Element::PickTrayByPDFSize:
      node = pdfium::MakeUnique<CXFA_PickTrayByPDFSize>(doc, packet);
      break;
    case XFA_Element::MonthNames:
      node = pdfium::MakeUnique<CXFA_MonthNames>(doc, packet);
      break;
    case XFA_Element::Severity:
      node = pdfium::MakeUnique<CXFA_Severity>(doc, packet);
      break;
    case XFA_Element::GroupParent:
      node = pdfium::MakeUnique<CXFA_GroupParent>(doc, packet);
      break;
    case XFA_Element::DocumentAssembly:
      node = pdfium::MakeUnique<CXFA_DocumentAssembly>(doc, packet);
      break;
    case XFA_Element::NumberSymbol:
      node = pdfium::MakeUnique<CXFA_NumberSymbol>(doc, packet);
      break;
    case XFA_Element::Tagged:
      node = pdfium::MakeUnique<CXFA_Tagged>(doc, packet);
      break;
    case XFA_Element::Items:
      node = pdfium::MakeUnique<CXFA_Items>(doc, packet);
      break;
    default:
      NOTREACHED();
      break;
  }

  return node && node->IsValidInPacket(packet) ? std::move(node) : nullptr;
}

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     uint16_t ePacket,
                     uint32_t validPackets,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     const PropertyData* properties,
                     const XFA_Attribute* attributes,
                     const WideStringView& elementName)
    : CXFA_Object(pDoc,
                  oType,
                  eType,
                  elementName,
                  pdfium::MakeUnique<CJX_Node>(this)),
      m_Properties(properties),
      m_Attributes(attributes),
      m_ValidPackets(validPackets),
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

bool CXFA_Node::IsValidInPacket(XFA_XDPPACKET packet) const {
  return !!(m_ValidPackets & packet);
}

const CXFA_Node::PropertyData* CXFA_Node::GetPropertyData(
    XFA_Element property) const {
  if (m_Properties == nullptr)
    return nullptr;

  for (size_t i = 0;; ++i) {
    const PropertyData* data = m_Properties + i;
    if (data->property == XFA_Element::Unknown)
      break;
    if (data->property == property)
      return data;
  }
  return nullptr;
}

bool CXFA_Node::HasProperty(XFA_Element property) const {
  return !!GetPropertyData(property);
}

bool CXFA_Node::HasPropertyFlags(XFA_Element property, uint8_t flags) const {
  const PropertyData* data = GetPropertyData(property);
  return data && !!(data->flags & flags);
}

uint8_t CXFA_Node::PropertyOccuranceCount(XFA_Element property) const {
  const PropertyData* data = GetPropertyData(property);
  return data ? data->occurance_count : 0;
}

std::pair<bool, XFA_Element> CXFA_Node::GetFirstPropertyWithFlag(uint8_t flag) {
  if (m_Properties == nullptr)
    return {false, XFA_Element::Unknown};

  for (size_t i = 0;; ++i) {
    const PropertyData* data = m_Properties + i;
    if (data->property == XFA_Element::Unknown)
      break;
    if (data->flags & flag)
      return {true, data->property};
  }
  return {false, XFA_Element::Unknown};
}

bool CXFA_Node::HasAttribute(XFA_Attribute attr) const {
  if (m_Attributes == nullptr)
    return false;

  for (size_t i = 0;; ++i) {
    XFA_Attribute cur_attr = *(m_Attributes + i);
    if (cur_attr == XFA_Attribute::Unknown)
      break;
    if (cur_attr == attr)
      return true;
  }
  return false;
}

// Note: This Method assumes that i is a valid index ....
XFA_Attribute CXFA_Node::GetAttribute(size_t i) const {
  if (m_Attributes == nullptr)
    return XFA_Attribute::Unknown;
  return *(m_Attributes + i);
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
      if (HasProperty(pChild->GetElementType())) {
        if (bFilterProperties) {
          nodes.push_back(pChild);
        } else if (bFilterOneOfProperties &&
                   HasPropertyFlags(pChild->GetElementType(),
                                    XFA_PROPERTYFLAG_OneOf)) {
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
      bool found;
      XFA_Element property;
      std::tie(found, property) =
          GetFirstPropertyWithFlag(XFA_PROPERTYFLAG_DefaultOneOf);
      if (found) {
        const XFA_PACKETINFO* pPacket = XFA_GetPacketByID(GetPacketID());
        CXFA_Node* pNewNode = m_pDocument->CreateNode(pPacket, property);
        if (pNewNode) {
          InsertChild(pNewNode, nullptr);
          pNewNode->SetFlag(XFA_NodeFlag_Initialized, true);
          nodes.push_back(pNewNode);
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
        if (HasProperty(pNode->GetElementType()))
          continue;
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
      if (bOnlyChild && HasProperty(pNode->GetElementType()))
        continue;

      iCount++;
      if (iCount > index)
        return pNode;
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


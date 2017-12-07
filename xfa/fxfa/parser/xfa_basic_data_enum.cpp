// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include "xfa/fxfa/fxfa_basic.h"

const XFA_AttributeEnumInfo g_XFAEnumData[] = {
    {0x2a, L"*", XFA_AttributeEnum::Asterisk},
    {0x2f, L"/", XFA_AttributeEnum::Slash},
    {0x5c, L"\\", XFA_AttributeEnum::Backslash},
    {0x239bd, L"on", XFA_AttributeEnum::On},
    {0x25356, L"tb", XFA_AttributeEnum::Tb},
    {0x25885, L"up", XFA_AttributeEnum::Up},
    {0x91b281, L"metaData", XFA_AttributeEnum::MetaData},
    {0x1f8dedb, L"delegate", XFA_AttributeEnum::Delegate},
    {0x2a6c55a, L"postSubmit", XFA_AttributeEnum::PostSubmit},
    {0x31b19c1, L"name", XFA_AttributeEnum::Name},
    {0x378a38a, L"cross", XFA_AttributeEnum::Cross},
    {0x3848b3f, L"next", XFA_AttributeEnum::Next},
    {0x48b6670, L"none", XFA_AttributeEnum::None},
    {0x51aafe5, L"shortEdge", XFA_AttributeEnum::ShortEdge},
    {0x55264c4, L"1mod10_1mod11", XFA_AttributeEnum::Checksum_1mod10_1mod11},
    {0x5a5c519, L"height", XFA_AttributeEnum::Height},
    {0x89ce549, L"crossDiagonal", XFA_AttributeEnum::CrossDiagonal},
    {0x9f9d0f9, L"all", XFA_AttributeEnum::All},
    {0x9f9db48, L"any", XFA_AttributeEnum::Any},
    {0xa126261, L"toRight", XFA_AttributeEnum::ToRight},
    {0xa36de29, L"matchTemplate", XFA_AttributeEnum::MatchTemplate},
    {0xa48d040, L"dpl", XFA_AttributeEnum::Dpl},
    {0xa559c05, L"invisible", XFA_AttributeEnum::Invisible},
    {0xa7d48e3, L"fit", XFA_AttributeEnum::Fit},
    {0xa8a8f80, L"width", XFA_AttributeEnum::Width},
    {0xab466bb, L"preSubmit", XFA_AttributeEnum::PreSubmit},
    {0xacc5785, L"ipl", XFA_AttributeEnum::Ipl},
    {0xafab0f8, L"flateCompress", XFA_AttributeEnum::FlateCompress},
    {0xb355816, L"med", XFA_AttributeEnum::Med},
    {0xb69ef77, L"odd", XFA_AttributeEnum::Odd},
    {0xb69f9bb, L"off", XFA_AttributeEnum::Off},
    {0xb843dba, L"pdf", XFA_AttributeEnum::Pdf},
    {0xbb912b8, L"row", XFA_AttributeEnum::Row},
    {0xbedaf33, L"top", XFA_AttributeEnum::Top},
    {0xc56afcc, L"xdp", XFA_AttributeEnum::Xdp},
    {0xc56ba02, L"xfd", XFA_AttributeEnum::Xfd},
    {0xc56ddf1, L"xml", XFA_AttributeEnum::Xml},
    {0xc8b65f3, L"zip", XFA_AttributeEnum::Zip},
    {0xc8b89d6, L"zpl", XFA_AttributeEnum::Zpl},
    {0xf55d7ee, L"visible", XFA_AttributeEnum::Visible},
    {0xfe3596a, L"exclude", XFA_AttributeEnum::Exclude},
    {0x109d7ce7, L"mouseEnter", XFA_AttributeEnum::MouseEnter},
    {0x10f1bc0c, L"pair", XFA_AttributeEnum::Pair},
    {0x1154efe6, L"filter", XFA_AttributeEnum::Filter},
    {0x125bc94b, L"moveLast", XFA_AttributeEnum::MoveLast},
    {0x12e1f1f0, L"exportAndImport", XFA_AttributeEnum::ExportAndImport},
    {0x13000c60, L"push", XFA_AttributeEnum::Push},
    {0x138ee315, L"portrait", XFA_AttributeEnum::Portrait},
    {0x14da2125, L"default", XFA_AttributeEnum::Default},
    {0x157749a5, L"storedProc", XFA_AttributeEnum::StoredProc},
    {0x16641198, L"stayBOF", XFA_AttributeEnum::StayBOF},
    {0x16b2fc5b, L"stayEOF", XFA_AttributeEnum::StayEOF},
    {0x17fad373, L"postPrint", XFA_AttributeEnum::PostPrint},
    {0x193207d0, L"usCarrier", XFA_AttributeEnum::UsCarrier},
    {0x193ade3e, L"right", XFA_AttributeEnum::Right},
    {0x1bfc72d9, L"preOpen", XFA_AttributeEnum::PreOpen},
    {0x1cc9317a, L"actual", XFA_AttributeEnum::Actual},
    {0x1f31df1e, L"rest", XFA_AttributeEnum::Rest},
    {0x1fb1bf14, L"topCenter", XFA_AttributeEnum::TopCenter},
    {0x207de667, L"standardSymbol", XFA_AttributeEnum::StandardSymbol},
    {0x2196a452, L"initialize", XFA_AttributeEnum::Initialize},
    {0x23bd40c7, L"justifyAll", XFA_AttributeEnum::JustifyAll},
    {0x247cf3e9, L"normal", XFA_AttributeEnum::Normal},
    {0x25aa946b, L"landscape", XFA_AttributeEnum::Landscape},
    {0x2739b5c9, L"nonInteractive", XFA_AttributeEnum::NonInteractive},
    {0x27410f03, L"mouseExit", XFA_AttributeEnum::MouseExit},
    {0x2854e62c, L"minus", XFA_AttributeEnum::Minus},
    {0x287e936a, L"diagonalLeft", XFA_AttributeEnum::DiagonalLeft},
    {0x2972a98f, L"simplexPaginated", XFA_AttributeEnum::SimplexPaginated},
    {0x29d8225f, L"document", XFA_AttributeEnum::Document},
    {0x2a9d3016, L"warning", XFA_AttributeEnum::Warning},
    {0x2b35b6d9, L"auto", XFA_AttributeEnum::Auto},
    {0x2c1653d9, L"below", XFA_AttributeEnum::Below},
    {0x2c1f0540, L"bottomLeft", XFA_AttributeEnum::BottomLeft},
    {0x2c44e816, L"bottomCenter", XFA_AttributeEnum::BottomCenter},
    {0x2cd3e9f3, L"tcpl", XFA_AttributeEnum::Tcpl},
    {0x2d08af85, L"text", XFA_AttributeEnum::Text},
    {0x2dc478eb, L"grouping", XFA_AttributeEnum::Grouping},
    {0x2ef3afdd, L"secureSymbol", XFA_AttributeEnum::SecureSymbol},
    {0x2f2dd29a, L"preExecute", XFA_AttributeEnum::PreExecute},
    {0x33c43dec, L"docClose", XFA_AttributeEnum::DocClose},
    {0x33f25bb5, L"keyset", XFA_AttributeEnum::Keyset},
    {0x34e363da, L"vertical", XFA_AttributeEnum::Vertical},
    {0x361fa1b6, L"preSave", XFA_AttributeEnum::PreSave},
    {0x36f1c6d8, L"preSign", XFA_AttributeEnum::PreSign},
    {0x399f02b5, L"bottom", XFA_AttributeEnum::Bottom},
    {0x3b0ab096, L"toTop", XFA_AttributeEnum::ToTop},
    {0x3c752495, L"verify", XFA_AttributeEnum::Verify},
    {0x3ce05d68, L"first", XFA_AttributeEnum::First},
    {0x3ecead94, L"contentArea", XFA_AttributeEnum::ContentArea},
    {0x40623b5b, L"solid", XFA_AttributeEnum::Solid},
    {0x42c6cd8d, L"pessimistic", XFA_AttributeEnum::Pessimistic},
    {0x43ddc6bf, L"duplexPaginated", XFA_AttributeEnum::DuplexPaginated},
    {0x442f68c8, L"round", XFA_AttributeEnum::Round},
    {0x45efb847, L"remerge", XFA_AttributeEnum::Remerge},
    {0x46972265, L"ordered", XFA_AttributeEnum::Ordered},
    {0x46f95531, L"percent", XFA_AttributeEnum::Percent},
    {0x46fd25ae, L"even", XFA_AttributeEnum::Even},
    {0x4731d6ba, L"exit", XFA_AttributeEnum::Exit},
    {0x4977356b, L"toolTip", XFA_AttributeEnum::ToolTip},
    {0x49b980ee, L"orderedOccurrence", XFA_AttributeEnum::OrderedOccurrence},
    {0x4a7e2dfe, L"readOnly", XFA_AttributeEnum::ReadOnly},
    {0x4c4e8acb, L"currency", XFA_AttributeEnum::Currency},
    {0x4dcf25f8, L"concat", XFA_AttributeEnum::Concat},
    {0x4febb826, L"Thai", XFA_AttributeEnum::Thai},
    {0x50ef95b2, L"embossed", XFA_AttributeEnum::Embossed},
    {0x516e35ce, L"formdata", XFA_AttributeEnum::Formdata},
    {0x52fa6f0e, L"Greek", XFA_AttributeEnum::Greek},
    {0x54034c2f, L"decimal", XFA_AttributeEnum::Decimal},
    {0x542c7300, L"select", XFA_AttributeEnum::Select},
    {0x551f0ae5, L"longEdge", XFA_AttributeEnum::LongEdge},
    {0x55520a8a, L"protected", XFA_AttributeEnum::Protected},
    {0x559f76f3, L"bottomRight", XFA_AttributeEnum::BottomRight},
    {0x568cb500, L"zero", XFA_AttributeEnum::Zero},
    {0x56bcecb7, L"forwardOnly", XFA_AttributeEnum::ForwardOnly},
    {0x56bf456b, L"docReady", XFA_AttributeEnum::DocReady},
    {0x573cb40c, L"hidden", XFA_AttributeEnum::Hidden},
    {0x582e3424, L"include", XFA_AttributeEnum::Include},
    {0x58a3dd29, L"dashed", XFA_AttributeEnum::Dashed},
    {0x5955b22b, L"multiSelect", XFA_AttributeEnum::MultiSelect},
    {0x598d5c53, L"inactive", XFA_AttributeEnum::Inactive},
    {0x59c8f27d, L"embed", XFA_AttributeEnum::Embed},
    {0x5e7555e8, L"static", XFA_AttributeEnum::Static},
    {0x606d4def, L"onEntry", XFA_AttributeEnum::OnEntry},
    {0x6195eafb, L"Cyrillic", XFA_AttributeEnum::Cyrillic},
    {0x6491b0f3, L"nonBlank", XFA_AttributeEnum::NonBlank},
    {0x67bef031, L"topRight", XFA_AttributeEnum::TopRight},
    {0x67df5ebd, L"Hebrew", XFA_AttributeEnum::Hebrew},
    {0x6aea98be, L"topLeft", XFA_AttributeEnum::TopLeft},
    {0x6c51afc1, L"center", XFA_AttributeEnum::Center},
    {0x7145e6bf, L"moveFirst", XFA_AttributeEnum::MoveFirst},
    {0x7375465c, L"diamond", XFA_AttributeEnum::Diamond},
    {0x7461aef4, L"pageOdd", XFA_AttributeEnum::PageOdd},
    {0x75f8aeb2, L"1mod10", XFA_AttributeEnum::Checksum_1mod10},
    {0x76d708e0, L"Korean", XFA_AttributeEnum::Korean},
    {0x789f14d7, L"aboveEmbedded", XFA_AttributeEnum::AboveEmbedded},
    {0x792ea39f, L"zipCompress", XFA_AttributeEnum::ZipCompress},
    {0x7a5b7193, L"numeric", XFA_AttributeEnum::Numeric},
    {0x7abec0d2, L"circle", XFA_AttributeEnum::Circle},
    {0x7afbba38, L"toBottom", XFA_AttributeEnum::ToBottom},
    {0x7b95e661, L"inverted", XFA_AttributeEnum::Inverted},
    {0x7baca2e3, L"update", XFA_AttributeEnum::Update},
    {0x7eb5da2c, L"isoname", XFA_AttributeEnum::Isoname},
    {0x7f6fd3d7, L"server", XFA_AttributeEnum::Server},
    {0x814f82b5, L"position", XFA_AttributeEnum::Position},
    {0x82deacf0, L"middleCenter", XFA_AttributeEnum::MiddleCenter},
    {0x83a49dc6, L"optional", XFA_AttributeEnum::Optional},
    {0x861a116f, L"usePrinterSetting", XFA_AttributeEnum::UsePrinterSetting},
    {0x86701ce0, L"outline", XFA_AttributeEnum::Outline},
    {0x8808385e, L"indexChange", XFA_AttributeEnum::IndexChange},
    {0x891f4606, L"change", XFA_AttributeEnum::Change},
    {0x89939f36, L"pageArea", XFA_AttributeEnum::PageArea},
    {0x8b5c3b25, L"once", XFA_AttributeEnum::Once},
    {0x8b5c6962, L"only", XFA_AttributeEnum::Only},
    {0x8b90e1f2, L"open", XFA_AttributeEnum::Open},
    {0x8bcfe96e, L"caption", XFA_AttributeEnum::Caption},
    {0x8ce83ef8, L"raised", XFA_AttributeEnum::Raised},
    {0x8d269cae, L"justify", XFA_AttributeEnum::Justify},
    {0x8fd520dc, L"refAndDescendants", XFA_AttributeEnum::RefAndDescendants},
    {0x9041d4b0, L"short", XFA_AttributeEnum::Short},
    {0x90c94426, L"pageFront", XFA_AttributeEnum::PageFront},
    {0x936beee5, L"monospace", XFA_AttributeEnum::Monospace},
    {0x947fa00f, L"middle", XFA_AttributeEnum::Middle},
    {0x9528a7b4, L"prePrint", XFA_AttributeEnum::PrePrint},
    {0x959ab231, L"always", XFA_AttributeEnum::Always},
    {0x96d61bf0, L"unknown", XFA_AttributeEnum::Unknown},
    {0x997194ee, L"toLeft", XFA_AttributeEnum::ToLeft},
    {0x9a83a3cd, L"above", XFA_AttributeEnum::Above},
    {0x9d0d71c7, L"dashDot", XFA_AttributeEnum::DashDot},
    {0x9df56f3e, L"gregorian", XFA_AttributeEnum::Gregorian},
    {0x9f6723fd, L"Roman", XFA_AttributeEnum::Roman},
    {0x9f693b21, L"mouseDown", XFA_AttributeEnum::MouseDown},
    {0xa1429b36, L"symbol", XFA_AttributeEnum::Symbol},
    {0xa5aa45cb, L"pageEven", XFA_AttributeEnum::PageEven},
    {0xa68635f1, L"sign", XFA_AttributeEnum::Sign},
    {0xa7315093, L"addNew", XFA_AttributeEnum::AddNew},
    {0xa7a773fa, L"star", XFA_AttributeEnum::Star},
    {0xa7d57b45, L"optimistic", XFA_AttributeEnum::Optimistic},
    {0xa8077321, L"rl-tb", XFA_AttributeEnum::Rl_tb},
    {0xa8f1468d, L"middleRight", XFA_AttributeEnum::MiddleRight},
    {0xaa84a1f1, L"maintain", XFA_AttributeEnum::Maintain},
    {0xab40b12c, L"package", XFA_AttributeEnum::Package},
    {0xac8b4d85, L"SimplifiedChinese", XFA_AttributeEnum::SimplifiedChinese},
    {0xadae6744, L"toCenter", XFA_AttributeEnum::ToCenter},
    {0xb0129df1, L"back", XFA_AttributeEnum::Back},
    {0xb0f088cf, L"unspecified", XFA_AttributeEnum::Unspecified},
    {0xb1271067, L"batchOptimistic", XFA_AttributeEnum::BatchOptimistic},
    {0xb18313a1, L"bold", XFA_AttributeEnum::Bold},
    {0xb1833cad, L"both", XFA_AttributeEnum::Both},
    {0xb221123f, L"butt", XFA_AttributeEnum::Butt},
    {0xb40c36bf, L"client", XFA_AttributeEnum::Client},
    {0xb56c7053, L"2mod10", XFA_AttributeEnum::Checksum_2mod10},
    {0xb683a345, L"imageOnly", XFA_AttributeEnum::ImageOnly},
    {0xb7732dea, L"horizontal", XFA_AttributeEnum::Horizontal},
    {0xb88652a4, L"dotted", XFA_AttributeEnum::Dotted},
    {0xbb2f2880, L"userControl", XFA_AttributeEnum::UserControl},
    {0xbbb79c5d, L"diagonalRight", XFA_AttributeEnum::DiagonalRight},
    {0xbd077154, L"consumeData", XFA_AttributeEnum::ConsumeData},
    {0xbd3fb11e, L"check", XFA_AttributeEnum::Check},
    {0xbde9abda, L"data", XFA_AttributeEnum::Data},
    {0xbf5a02d8, L"down", XFA_AttributeEnum::Down},
    {0xbf7450ee, L"sansSerif", XFA_AttributeEnum::SansSerif},
    {0xc02d649f, L"inline", XFA_AttributeEnum::Inline},
    {0xc11a9e3a, L"TraditionalChinese", XFA_AttributeEnum::TraditionalChinese},
    {0xc16169d8, L"warn", XFA_AttributeEnum::Warn},
    {0xc16f071f, L"refOnly", XFA_AttributeEnum::RefOnly},
    {0xc27c8ba5, L"interactiveForms", XFA_AttributeEnum::InteractiveForms},
    {0xc2d1b15c, L"word", XFA_AttributeEnum::Word},
    {0xc3621288, L"unordered", XFA_AttributeEnum::Unordered},
    {0xc5251981, L"required", XFA_AttributeEnum::Required},
    {0xc7088e7d, L"importOnly", XFA_AttributeEnum::ImportOnly},
    {0xc72cf0e3, L"belowEmbedded", XFA_AttributeEnum::BelowEmbedded},
    {0xc819cf07, L"Japanese", XFA_AttributeEnum::Japanese},
    {0xcdce56b3, L"full", XFA_AttributeEnum::Full},
    {0xce0122e3, L"rl-row", XFA_AttributeEnum::Rl_row},
    {0xcf7d71f1, L"Vietnamese", XFA_AttributeEnum::Vietnamese},
    {0xcfde3e09, L"EastEuropeanRoman", XFA_AttributeEnum::EastEuropeanRoman},
    {0xd576d08e, L"mouseUp", XFA_AttributeEnum::MouseUp},
    {0xd7a92904, L"exportOnly", XFA_AttributeEnum::ExportOnly},
    {0xd8ed1467, L"clear", XFA_AttributeEnum::Clear},
    {0xd95657a6, L"click", XFA_AttributeEnum::Click},
    {0xd96c7de5, L"base64", XFA_AttributeEnum::Base64},
    {0xd9f47f36, L"close", XFA_AttributeEnum::Close},
    {0xdb075bde, L"host", XFA_AttributeEnum::Host},
    {0xdb103411, L"global", XFA_AttributeEnum::Global},
    {0xdb647188, L"blank", XFA_AttributeEnum::Blank},
    {0xdb9be968, L"table", XFA_AttributeEnum::Table},
    {0xdf590fbb, L"import", XFA_AttributeEnum::Import},
    {0xe0e573fb, L"custom", XFA_AttributeEnum::Custom},
    {0xe0ecc79a, L"middleLeft", XFA_AttributeEnum::MiddleLeft},
    {0xe1452019, L"postExecute", XFA_AttributeEnum::PostExecute},
    {0xe1911d98, L"radix", XFA_AttributeEnum::Radix},
    {0xe25fa7b8, L"postOpen", XFA_AttributeEnum::PostOpen},
    {0xe28dce7e, L"enter", XFA_AttributeEnum::Enter},
    {0xe2c44de4, L"ignore", XFA_AttributeEnum::Ignore},
    {0xe2cd8c61, L"lr-tb", XFA_AttributeEnum::Lr_tb},
    {0xe2da8336, L"fantasy", XFA_AttributeEnum::Fantasy},
    {0xe31d5396, L"italic", XFA_AttributeEnum::Italic},
    {0xe7ada113, L"author", XFA_AttributeEnum::Author},
    {0xe8e7cc18, L"toEdge", XFA_AttributeEnum::ToEdge},
    {0xe97aa98b, L"choice", XFA_AttributeEnum::Choice},
    {0xeafd2a38, L"disabled", XFA_AttributeEnum::Disabled},
    {0xeb2b7972, L"crossHatch", XFA_AttributeEnum::CrossHatch},
    {0xeb2db2d7, L"dataRef", XFA_AttributeEnum::DataRef},
    {0xec35dc6e, L"dashDotDot", XFA_AttributeEnum::DashDotDot},
    {0xef85d351, L"square", XFA_AttributeEnum::Square},
    {0xf2102445, L"dynamic", XFA_AttributeEnum::Dynamic},
    {0xf272c7be, L"manual", XFA_AttributeEnum::Manual},
    {0xf2bbb64d, L"etched", XFA_AttributeEnum::Etched},
    {0xf3b8fc6c, L"validationState", XFA_AttributeEnum::ValidationState},
    {0xf42f2b81, L"cursive", XFA_AttributeEnum::Cursive},
    {0xf54481d4, L"last", XFA_AttributeEnum::Last},
    {0xf5ad782b, L"left", XFA_AttributeEnum::Left},
    {0xf616da2e, L"link", XFA_AttributeEnum::Link},
    {0xf6b4afb0, L"long", XFA_AttributeEnum::Long},
    {0xf8636460, L"internationalCarrier",
     XFA_AttributeEnum::InternationalCarrier},
    {0xf9fb37ac, L"PDF1.3", XFA_AttributeEnum::PDF1_3},
    {0xf9fb37af, L"PDF1.6", XFA_AttributeEnum::PDF1_6},
    {0xfbce7f19, L"serif", XFA_AttributeEnum::Serif},
    {0xfc82d695, L"postSave", XFA_AttributeEnum::PostSave},
    {0xfcef86b5, L"ready", XFA_AttributeEnum::Ready},
    {0xfd54fbb7, L"postSign", XFA_AttributeEnum::PostSign},
    {0xfdc0aae2, L"Arabic", XFA_AttributeEnum::Arabic},
    {0xfe06d2ca, L"error", XFA_AttributeEnum::Error},
    {0xfefc4885, L"urlencoded", XFA_AttributeEnum::Urlencoded},
    {0xff795ad2, L"lowered", XFA_AttributeEnum::Lowered},
};
const int32_t g_iXFAEnumCount =
    sizeof(g_XFAEnumData) / sizeof(XFA_AttributeEnumInfo);

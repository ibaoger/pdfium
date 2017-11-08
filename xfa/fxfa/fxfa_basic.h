// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FXFA_BASIC_H_
#define XFA_FXFA_FXFA_BASIC_H_

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/fxjse.h"

class CFXJSE_Arguments;
class CJX_Object;
class CXFA_Measurement;
enum class XFA_ObjectType;

enum XFA_HashCode : uint32_t {
  XFA_HASHCODE_None = 0,

  XFA_HASHCODE_Config = 0x4e1e39b6,
  XFA_HASHCODE_ConnectionSet = 0xe14c801c,
  XFA_HASHCODE_Data = 0xbde9abda,
  XFA_HASHCODE_DataDescription = 0x2b5df51e,
  XFA_HASHCODE_Datasets = 0x99b95079,
  XFA_HASHCODE_DataWindow = 0x83a550d2,
  XFA_HASHCODE_Event = 0x185e41e2,
  XFA_HASHCODE_Form = 0xcd309ff4,
  XFA_HASHCODE_Group = 0xf7f75fcd,
  XFA_HASHCODE_Host = 0xdb075bde,
  XFA_HASHCODE_Layout = 0x7e7e845e,
  XFA_HASHCODE_LocaleSet = 0x5473b6dc,
  XFA_HASHCODE_Log = 0x0b1b3d22,
  XFA_HASHCODE_Name = 0x31b19c1,
  XFA_HASHCODE_Occur = 0xf7eebe1c,
  XFA_HASHCODE_Pdf = 0xb843dba,
  XFA_HASHCODE_Record = 0x5779d65f,
  XFA_HASHCODE_Signature = 0x8b036f32,
  XFA_HASHCODE_SourceSet = 0x811929d,
  XFA_HASHCODE_Stylesheet = 0x6038580a,
  XFA_HASHCODE_Template = 0x803550fc,
  XFA_HASHCODE_This = 0x2d574d58,
  XFA_HASHCODE_Xdc = 0xc56afbf,
  XFA_HASHCODE_XDP = 0xc56afcc,
  XFA_HASHCODE_Xfa = 0xc56b9ff,
  XFA_HASHCODE_Xfdf = 0x48d004a8,
  XFA_HASHCODE_Xmpmeta = 0x132a8fbc
};

enum XFA_PACKET {
  XFA_PACKET_USER,
  XFA_PACKET_SourceSet,
  XFA_PACKET_Pdf,
  XFA_PACKET_Xdc,
  XFA_PACKET_XDP,
  XFA_PACKET_Xmpmeta,
  XFA_PACKET_Xfdf,
  XFA_PACKET_Config,
  XFA_PACKET_LocaleSet,
  XFA_PACKET_Stylesheet,
  XFA_PACKET_Template,
  XFA_PACKET_Signature,
  XFA_PACKET_Datasets,
  XFA_PACKET_Form,
  XFA_PACKET_ConnectionSet,
};

enum XFA_XDPPACKET {
  XFA_XDPPACKET_UNKNOWN = 0,
  XFA_XDPPACKET_Config = 1 << XFA_PACKET_Config,
  XFA_XDPPACKET_Template = 1 << XFA_PACKET_Template,
  XFA_XDPPACKET_Datasets = 1 << XFA_PACKET_Datasets,
  XFA_XDPPACKET_Form = 1 << XFA_PACKET_Form,
  XFA_XDPPACKET_LocaleSet = 1 << XFA_PACKET_LocaleSet,
  XFA_XDPPACKET_ConnectionSet = 1 << XFA_PACKET_ConnectionSet,
  XFA_XDPPACKET_SourceSet = 1 << XFA_PACKET_SourceSet,
  XFA_XDPPACKET_Xdc = 1 << XFA_PACKET_Xdc,
  XFA_XDPPACKET_Pdf = 1 << XFA_PACKET_Pdf,
  XFA_XDPPACKET_Xfdf = 1 << XFA_PACKET_Xfdf,
  XFA_XDPPACKET_Xmpmeta = 1 << XFA_PACKET_Xmpmeta,
  XFA_XDPPACKET_Signature = 1 << XFA_PACKET_Signature,
  XFA_XDPPACKET_Stylesheet = 1 << XFA_PACKET_Stylesheet,
  XFA_XDPPACKET_USER = 1 << XFA_PACKET_USER,
  XFA_XDPPACKET_XDP = 1 << XFA_PACKET_XDP,
};
enum XFA_XDPPACKET_FLAGS {
  XFA_XDPPACKET_FLAGS_COMPLETEMATCH = 1,
  XFA_XDPPACKET_FLAGS_PREFIXMATCH = 2,
  XFA_XDPPACKET_FLAGS_NOMATCH = 4,
  XFA_XDPPACKET_FLAGS_SUPPORTONE = 8,
  XFA_XDPPACKET_FLAGS_SUPPORTMANY = 16,
};
struct XFA_PACKETINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_XDPPACKET eName;
  const wchar_t* pURI;
  uint32_t eFlags;
};

enum XFA_ATTRIBUTEENUM {
  XFA_ATTRIBUTEENUM_Asterisk,
  XFA_ATTRIBUTEENUM_Slash,
  XFA_ATTRIBUTEENUM_Backslash,
  XFA_ATTRIBUTEENUM_On,
  XFA_ATTRIBUTEENUM_Tb,
  XFA_ATTRIBUTEENUM_Up,
  XFA_ATTRIBUTEENUM_MetaData,
  XFA_ATTRIBUTEENUM_Delegate,
  XFA_ATTRIBUTEENUM_PostSubmit,
  XFA_ATTRIBUTEENUM_Name,
  XFA_ATTRIBUTEENUM_Cross,
  XFA_ATTRIBUTEENUM_Next,
  XFA_ATTRIBUTEENUM_None,
  XFA_ATTRIBUTEENUM_ShortEdge,
  XFA_ATTRIBUTEENUM_1mod10_1mod11,
  XFA_ATTRIBUTEENUM_Height,
  XFA_ATTRIBUTEENUM_CrossDiagonal,
  XFA_ATTRIBUTEENUM_All,
  XFA_ATTRIBUTEENUM_Any,
  XFA_ATTRIBUTEENUM_ToRight,
  XFA_ATTRIBUTEENUM_MatchTemplate,
  XFA_ATTRIBUTEENUM_Dpl,
  XFA_ATTRIBUTEENUM_Invisible,
  XFA_ATTRIBUTEENUM_Fit,
  XFA_ATTRIBUTEENUM_Width,
  XFA_ATTRIBUTEENUM_PreSubmit,
  XFA_ATTRIBUTEENUM_Ipl,
  XFA_ATTRIBUTEENUM_FlateCompress,
  XFA_ATTRIBUTEENUM_Med,
  XFA_ATTRIBUTEENUM_Odd,
  XFA_ATTRIBUTEENUM_Off,
  XFA_ATTRIBUTEENUM_Pdf,
  XFA_ATTRIBUTEENUM_Row,
  XFA_ATTRIBUTEENUM_Top,
  XFA_ATTRIBUTEENUM_Xdp,
  XFA_ATTRIBUTEENUM_Xfd,
  XFA_ATTRIBUTEENUM_Xml,
  XFA_ATTRIBUTEENUM_Zip,
  XFA_ATTRIBUTEENUM_Zpl,
  XFA_ATTRIBUTEENUM_Visible,
  XFA_ATTRIBUTEENUM_Exclude,
  XFA_ATTRIBUTEENUM_MouseEnter,
  XFA_ATTRIBUTEENUM_Pair,
  XFA_ATTRIBUTEENUM_Filter,
  XFA_ATTRIBUTEENUM_MoveLast,
  XFA_ATTRIBUTEENUM_ExportAndImport,
  XFA_ATTRIBUTEENUM_Push,
  XFA_ATTRIBUTEENUM_Portrait,
  XFA_ATTRIBUTEENUM_Default,
  XFA_ATTRIBUTEENUM_StoredProc,
  XFA_ATTRIBUTEENUM_StayBOF,
  XFA_ATTRIBUTEENUM_StayEOF,
  XFA_ATTRIBUTEENUM_PostPrint,
  XFA_ATTRIBUTEENUM_UsCarrier,
  XFA_ATTRIBUTEENUM_Right,
  XFA_ATTRIBUTEENUM_PreOpen,
  XFA_ATTRIBUTEENUM_Actual,
  XFA_ATTRIBUTEENUM_Rest,
  XFA_ATTRIBUTEENUM_TopCenter,
  XFA_ATTRIBUTEENUM_StandardSymbol,
  XFA_ATTRIBUTEENUM_Initialize,
  XFA_ATTRIBUTEENUM_JustifyAll,
  XFA_ATTRIBUTEENUM_Normal,
  XFA_ATTRIBUTEENUM_Landscape,
  XFA_ATTRIBUTEENUM_NonInteractive,
  XFA_ATTRIBUTEENUM_MouseExit,
  XFA_ATTRIBUTEENUM_Minus,
  XFA_ATTRIBUTEENUM_DiagonalLeft,
  XFA_ATTRIBUTEENUM_SimplexPaginated,
  XFA_ATTRIBUTEENUM_Document,
  XFA_ATTRIBUTEENUM_Warning,
  XFA_ATTRIBUTEENUM_Auto,
  XFA_ATTRIBUTEENUM_Below,
  XFA_ATTRIBUTEENUM_BottomLeft,
  XFA_ATTRIBUTEENUM_BottomCenter,
  XFA_ATTRIBUTEENUM_Tcpl,
  XFA_ATTRIBUTEENUM_Text,
  XFA_ATTRIBUTEENUM_Grouping,
  XFA_ATTRIBUTEENUM_SecureSymbol,
  XFA_ATTRIBUTEENUM_PreExecute,
  XFA_ATTRIBUTEENUM_DocClose,
  XFA_ATTRIBUTEENUM_Keyset,
  XFA_ATTRIBUTEENUM_Vertical,
  XFA_ATTRIBUTEENUM_PreSave,
  XFA_ATTRIBUTEENUM_PreSign,
  XFA_ATTRIBUTEENUM_Bottom,
  XFA_ATTRIBUTEENUM_ToTop,
  XFA_ATTRIBUTEENUM_Verify,
  XFA_ATTRIBUTEENUM_First,
  XFA_ATTRIBUTEENUM_ContentArea,
  XFA_ATTRIBUTEENUM_Solid,
  XFA_ATTRIBUTEENUM_Pessimistic,
  XFA_ATTRIBUTEENUM_DuplexPaginated,
  XFA_ATTRIBUTEENUM_Round,
  XFA_ATTRIBUTEENUM_Remerge,
  XFA_ATTRIBUTEENUM_Ordered,
  XFA_ATTRIBUTEENUM_Percent,
  XFA_ATTRIBUTEENUM_Even,
  XFA_ATTRIBUTEENUM_Exit,
  XFA_ATTRIBUTEENUM_ToolTip,
  XFA_ATTRIBUTEENUM_OrderedOccurrence,
  XFA_ATTRIBUTEENUM_ReadOnly,
  XFA_ATTRIBUTEENUM_Currency,
  XFA_ATTRIBUTEENUM_Concat,
  XFA_ATTRIBUTEENUM_Thai,
  XFA_ATTRIBUTEENUM_Embossed,
  XFA_ATTRIBUTEENUM_Formdata,
  XFA_ATTRIBUTEENUM_Greek,
  XFA_ATTRIBUTEENUM_Decimal,
  XFA_ATTRIBUTEENUM_Select,
  XFA_ATTRIBUTEENUM_LongEdge,
  XFA_ATTRIBUTEENUM_Protected,
  XFA_ATTRIBUTEENUM_BottomRight,
  XFA_ATTRIBUTEENUM_Zero,
  XFA_ATTRIBUTEENUM_ForwardOnly,
  XFA_ATTRIBUTEENUM_DocReady,
  XFA_ATTRIBUTEENUM_Hidden,
  XFA_ATTRIBUTEENUM_Include,
  XFA_ATTRIBUTEENUM_Dashed,
  XFA_ATTRIBUTEENUM_MultiSelect,
  XFA_ATTRIBUTEENUM_Inactive,
  XFA_ATTRIBUTEENUM_Embed,
  XFA_ATTRIBUTEENUM_Static,
  XFA_ATTRIBUTEENUM_OnEntry,
  XFA_ATTRIBUTEENUM_Cyrillic,
  XFA_ATTRIBUTEENUM_NonBlank,
  XFA_ATTRIBUTEENUM_TopRight,
  XFA_ATTRIBUTEENUM_Hebrew,
  XFA_ATTRIBUTEENUM_TopLeft,
  XFA_ATTRIBUTEENUM_Center,
  XFA_ATTRIBUTEENUM_MoveFirst,
  XFA_ATTRIBUTEENUM_Diamond,
  XFA_ATTRIBUTEENUM_PageOdd,
  XFA_ATTRIBUTEENUM_1mod10,
  XFA_ATTRIBUTEENUM_Korean,
  XFA_ATTRIBUTEENUM_AboveEmbedded,
  XFA_ATTRIBUTEENUM_ZipCompress,
  XFA_ATTRIBUTEENUM_Numeric,
  XFA_ATTRIBUTEENUM_Circle,
  XFA_ATTRIBUTEENUM_ToBottom,
  XFA_ATTRIBUTEENUM_Inverted,
  XFA_ATTRIBUTEENUM_Update,
  XFA_ATTRIBUTEENUM_Isoname,
  XFA_ATTRIBUTEENUM_Server,
  XFA_ATTRIBUTEENUM_Position,
  XFA_ATTRIBUTEENUM_MiddleCenter,
  XFA_ATTRIBUTEENUM_Optional,
  XFA_ATTRIBUTEENUM_UsePrinterSetting,
  XFA_ATTRIBUTEENUM_Outline,
  XFA_ATTRIBUTEENUM_IndexChange,
  XFA_ATTRIBUTEENUM_Change,
  XFA_ATTRIBUTEENUM_PageArea,
  XFA_ATTRIBUTEENUM_Once,
  XFA_ATTRIBUTEENUM_Only,
  XFA_ATTRIBUTEENUM_Open,
  XFA_ATTRIBUTEENUM_Caption,
  XFA_ATTRIBUTEENUM_Raised,
  XFA_ATTRIBUTEENUM_Justify,
  XFA_ATTRIBUTEENUM_RefAndDescendants,
  XFA_ATTRIBUTEENUM_Short,
  XFA_ATTRIBUTEENUM_PageFront,
  XFA_ATTRIBUTEENUM_Monospace,
  XFA_ATTRIBUTEENUM_Middle,
  XFA_ATTRIBUTEENUM_PrePrint,
  XFA_ATTRIBUTEENUM_Always,
  XFA_ATTRIBUTEENUM_Unknown,
  XFA_ATTRIBUTEENUM_ToLeft,
  XFA_ATTRIBUTEENUM_Above,
  XFA_ATTRIBUTEENUM_DashDot,
  XFA_ATTRIBUTEENUM_Gregorian,
  XFA_ATTRIBUTEENUM_Roman,
  XFA_ATTRIBUTEENUM_MouseDown,
  XFA_ATTRIBUTEENUM_Symbol,
  XFA_ATTRIBUTEENUM_PageEven,
  XFA_ATTRIBUTEENUM_Sign,
  XFA_ATTRIBUTEENUM_AddNew,
  XFA_ATTRIBUTEENUM_Star,
  XFA_ATTRIBUTEENUM_Optimistic,
  XFA_ATTRIBUTEENUM_Rl_tb,
  XFA_ATTRIBUTEENUM_MiddleRight,
  XFA_ATTRIBUTEENUM_Maintain,
  XFA_ATTRIBUTEENUM_Package,
  XFA_ATTRIBUTEENUM_SimplifiedChinese,
  XFA_ATTRIBUTEENUM_ToCenter,
  XFA_ATTRIBUTEENUM_Back,
  XFA_ATTRIBUTEENUM_Unspecified,
  XFA_ATTRIBUTEENUM_BatchOptimistic,
  XFA_ATTRIBUTEENUM_Bold,
  XFA_ATTRIBUTEENUM_Both,
  XFA_ATTRIBUTEENUM_Butt,
  XFA_ATTRIBUTEENUM_Client,
  XFA_ATTRIBUTEENUM_2mod10,
  XFA_ATTRIBUTEENUM_ImageOnly,
  XFA_ATTRIBUTEENUM_Horizontal,
  XFA_ATTRIBUTEENUM_Dotted,
  XFA_ATTRIBUTEENUM_UserControl,
  XFA_ATTRIBUTEENUM_DiagonalRight,
  XFA_ATTRIBUTEENUM_ConsumeData,
  XFA_ATTRIBUTEENUM_Check,
  XFA_ATTRIBUTEENUM_Data,
  XFA_ATTRIBUTEENUM_Down,
  XFA_ATTRIBUTEENUM_SansSerif,
  XFA_ATTRIBUTEENUM_Inline,
  XFA_ATTRIBUTEENUM_TraditionalChinese,
  XFA_ATTRIBUTEENUM_Warn,
  XFA_ATTRIBUTEENUM_RefOnly,
  XFA_ATTRIBUTEENUM_InteractiveForms,
  XFA_ATTRIBUTEENUM_Word,
  XFA_ATTRIBUTEENUM_Unordered,
  XFA_ATTRIBUTEENUM_Required,
  XFA_ATTRIBUTEENUM_ImportOnly,
  XFA_ATTRIBUTEENUM_BelowEmbedded,
  XFA_ATTRIBUTEENUM_Japanese,
  XFA_ATTRIBUTEENUM_Full,
  XFA_ATTRIBUTEENUM_Rl_row,
  XFA_ATTRIBUTEENUM_Vietnamese,
  XFA_ATTRIBUTEENUM_EastEuropeanRoman,
  XFA_ATTRIBUTEENUM_MouseUp,
  XFA_ATTRIBUTEENUM_ExportOnly,
  XFA_ATTRIBUTEENUM_Clear,
  XFA_ATTRIBUTEENUM_Click,
  XFA_ATTRIBUTEENUM_Base64,
  XFA_ATTRIBUTEENUM_Close,
  XFA_ATTRIBUTEENUM_Host,
  XFA_ATTRIBUTEENUM_Global,
  XFA_ATTRIBUTEENUM_Blank,
  XFA_ATTRIBUTEENUM_Table,
  XFA_ATTRIBUTEENUM_Import,
  XFA_ATTRIBUTEENUM_Custom,
  XFA_ATTRIBUTEENUM_MiddleLeft,
  XFA_ATTRIBUTEENUM_PostExecute,
  XFA_ATTRIBUTEENUM_Radix,
  XFA_ATTRIBUTEENUM_PostOpen,
  XFA_ATTRIBUTEENUM_Enter,
  XFA_ATTRIBUTEENUM_Ignore,
  XFA_ATTRIBUTEENUM_Lr_tb,
  XFA_ATTRIBUTEENUM_Fantasy,
  XFA_ATTRIBUTEENUM_Italic,
  XFA_ATTRIBUTEENUM_Author,
  XFA_ATTRIBUTEENUM_ToEdge,
  XFA_ATTRIBUTEENUM_Choice,
  XFA_ATTRIBUTEENUM_Disabled,
  XFA_ATTRIBUTEENUM_CrossHatch,
  XFA_ATTRIBUTEENUM_DataRef,
  XFA_ATTRIBUTEENUM_DashDotDot,
  XFA_ATTRIBUTEENUM_Square,
  XFA_ATTRIBUTEENUM_Dynamic,
  XFA_ATTRIBUTEENUM_Manual,
  XFA_ATTRIBUTEENUM_Etched,
  XFA_ATTRIBUTEENUM_ValidationState,
  XFA_ATTRIBUTEENUM_Cursive,
  XFA_ATTRIBUTEENUM_Last,
  XFA_ATTRIBUTEENUM_Left,
  XFA_ATTRIBUTEENUM_Link,
  XFA_ATTRIBUTEENUM_Long,
  XFA_ATTRIBUTEENUM_InternationalCarrier,
  XFA_ATTRIBUTEENUM_PDF1_3,
  XFA_ATTRIBUTEENUM_PDF1_6,
  XFA_ATTRIBUTEENUM_Serif,
  XFA_ATTRIBUTEENUM_PostSave,
  XFA_ATTRIBUTEENUM_Ready,
  XFA_ATTRIBUTEENUM_PostSign,
  XFA_ATTRIBUTEENUM_Arabic,
  XFA_ATTRIBUTEENUM_Error,
  XFA_ATTRIBUTEENUM_Urlencoded,
  XFA_ATTRIBUTEENUM_Lowered,
};
enum XFA_ATTRIBUTE {
  XFA_ATTRIBUTE_Unknown = -1,

  XFA_ATTRIBUTE_H,
  XFA_ATTRIBUTE_W,
  XFA_ATTRIBUTE_X,
  XFA_ATTRIBUTE_Y,
  XFA_ATTRIBUTE_Id,
  XFA_ATTRIBUTE_To,
  XFA_ATTRIBUTE_LineThrough,
  XFA_ATTRIBUTE_HAlign,
  XFA_ATTRIBUTE_Typeface,
  XFA_ATTRIBUTE_BeforeTarget,
  XFA_ATTRIBUTE_Name,
  XFA_ATTRIBUTE_Next,
  XFA_ATTRIBUTE_DataRowCount,
  XFA_ATTRIBUTE_Break,
  XFA_ATTRIBUTE_VScrollPolicy,
  XFA_ATTRIBUTE_FontHorizontalScale,
  XFA_ATTRIBUTE_TextIndent,
  XFA_ATTRIBUTE_Context,
  XFA_ATTRIBUTE_TrayOut,
  XFA_ATTRIBUTE_Cap,
  XFA_ATTRIBUTE_Max,
  XFA_ATTRIBUTE_Min,
  XFA_ATTRIBUTE_Ref,
  XFA_ATTRIBUTE_Rid,
  XFA_ATTRIBUTE_Url,
  XFA_ATTRIBUTE_Use,
  XFA_ATTRIBUTE_LeftInset,
  XFA_ATTRIBUTE_Widows,
  XFA_ATTRIBUTE_Level,
  XFA_ATTRIBUTE_BottomInset,
  XFA_ATTRIBUTE_OverflowTarget,
  XFA_ATTRIBUTE_AllowMacro,
  XFA_ATTRIBUTE_PagePosition,
  XFA_ATTRIBUTE_ColumnWidths,
  XFA_ATTRIBUTE_OverflowLeader,
  XFA_ATTRIBUTE_Action,
  XFA_ATTRIBUTE_NonRepudiation,
  XFA_ATTRIBUTE_Rate,
  XFA_ATTRIBUTE_AllowRichText,
  XFA_ATTRIBUTE_Role,
  XFA_ATTRIBUTE_OverflowTrailer,
  XFA_ATTRIBUTE_Operation,
  XFA_ATTRIBUTE_Timeout,
  XFA_ATTRIBUTE_TopInset,
  XFA_ATTRIBUTE_Access,
  XFA_ATTRIBUTE_CommandType,
  XFA_ATTRIBUTE_Format,
  XFA_ATTRIBUTE_DataPrep,
  XFA_ATTRIBUTE_WidgetData,
  XFA_ATTRIBUTE_Abbr,
  XFA_ATTRIBUTE_MarginRight,
  XFA_ATTRIBUTE_DataDescription,
  XFA_ATTRIBUTE_EncipherOnly,
  XFA_ATTRIBUTE_KerningMode,
  XFA_ATTRIBUTE_Rotate,
  XFA_ATTRIBUTE_WordCharacterCount,
  XFA_ATTRIBUTE_Type,
  XFA_ATTRIBUTE_Reserve,
  XFA_ATTRIBUTE_TextLocation,
  XFA_ATTRIBUTE_Priority,
  XFA_ATTRIBUTE_Underline,
  XFA_ATTRIBUTE_ModuleWidth,
  XFA_ATTRIBUTE_Hyphenate,
  XFA_ATTRIBUTE_Listen,
  XFA_ATTRIBUTE_Delimiter,
  XFA_ATTRIBUTE_ContentType,
  XFA_ATTRIBUTE_StartNew,
  XFA_ATTRIBUTE_EofAction,
  XFA_ATTRIBUTE_AllowNeutral,
  XFA_ATTRIBUTE_Connection,
  XFA_ATTRIBUTE_BaselineShift,
  XFA_ATTRIBUTE_OverlinePeriod,
  XFA_ATTRIBUTE_FracDigits,
  XFA_ATTRIBUTE_Orientation,
  XFA_ATTRIBUTE_TimeStamp,
  XFA_ATTRIBUTE_PrintCheckDigit,
  XFA_ATTRIBUTE_MarginLeft,
  XFA_ATTRIBUTE_Stroke,
  XFA_ATTRIBUTE_ModuleHeight,
  XFA_ATTRIBUTE_TransferEncoding,
  XFA_ATTRIBUTE_Usage,
  XFA_ATTRIBUTE_Presence,
  XFA_ATTRIBUTE_RadixOffset,
  XFA_ATTRIBUTE_Preserve,
  XFA_ATTRIBUTE_AliasNode,
  XFA_ATTRIBUTE_MultiLine,
  XFA_ATTRIBUTE_Version,
  XFA_ATTRIBUTE_StartChar,
  XFA_ATTRIBUTE_ScriptTest,
  XFA_ATTRIBUTE_StartAngle,
  XFA_ATTRIBUTE_CursorType,
  XFA_ATTRIBUTE_DigitalSignature,
  XFA_ATTRIBUTE_CodeType,
  XFA_ATTRIBUTE_Output,
  XFA_ATTRIBUTE_BookendTrailer,
  XFA_ATTRIBUTE_ImagingBBox,
  XFA_ATTRIBUTE_ExcludeInitialCap,
  XFA_ATTRIBUTE_Force,
  XFA_ATTRIBUTE_CrlSign,
  XFA_ATTRIBUTE_Previous,
  XFA_ATTRIBUTE_PushCharacterCount,
  XFA_ATTRIBUTE_NullTest,
  XFA_ATTRIBUTE_RunAt,
  XFA_ATTRIBUTE_SpaceBelow,
  XFA_ATTRIBUTE_SweepAngle,
  XFA_ATTRIBUTE_NumberOfCells,
  XFA_ATTRIBUTE_LetterSpacing,
  XFA_ATTRIBUTE_LockType,
  XFA_ATTRIBUTE_PasswordChar,
  XFA_ATTRIBUTE_VAlign,
  XFA_ATTRIBUTE_SourceBelow,
  XFA_ATTRIBUTE_Inverted,
  XFA_ATTRIBUTE_Mark,
  XFA_ATTRIBUTE_MaxH,
  XFA_ATTRIBUTE_MaxW,
  XFA_ATTRIBUTE_Truncate,
  XFA_ATTRIBUTE_MinH,
  XFA_ATTRIBUTE_MinW,
  XFA_ATTRIBUTE_Initial,
  XFA_ATTRIBUTE_Mode,
  XFA_ATTRIBUTE_Layout,
  XFA_ATTRIBUTE_Server,
  XFA_ATTRIBUTE_EmbedPDF,
  XFA_ATTRIBUTE_OddOrEven,
  XFA_ATTRIBUTE_TabDefault,
  XFA_ATTRIBUTE_Contains,
  XFA_ATTRIBUTE_RightInset,
  XFA_ATTRIBUTE_MaxChars,
  XFA_ATTRIBUTE_Open,
  XFA_ATTRIBUTE_Relation,
  XFA_ATTRIBUTE_WideNarrowRatio,
  XFA_ATTRIBUTE_Relevant,
  XFA_ATTRIBUTE_SignatureType,
  XFA_ATTRIBUTE_LineThroughPeriod,
  XFA_ATTRIBUTE_Shape,
  XFA_ATTRIBUTE_TabStops,
  XFA_ATTRIBUTE_OutputBelow,
  XFA_ATTRIBUTE_Short,
  XFA_ATTRIBUTE_FontVerticalScale,
  XFA_ATTRIBUTE_Thickness,
  XFA_ATTRIBUTE_CommitOn,
  XFA_ATTRIBUTE_RemainCharacterCount,
  XFA_ATTRIBUTE_KeyAgreement,
  XFA_ATTRIBUTE_ErrorCorrectionLevel,
  XFA_ATTRIBUTE_UpsMode,
  XFA_ATTRIBUTE_MergeMode,
  XFA_ATTRIBUTE_Circular,
  XFA_ATTRIBUTE_PsName,
  XFA_ATTRIBUTE_Trailer,
  XFA_ATTRIBUTE_UnicodeRange,
  XFA_ATTRIBUTE_ExecuteType,
  XFA_ATTRIBUTE_DuplexImposition,
  XFA_ATTRIBUTE_TrayIn,
  XFA_ATTRIBUTE_BindingNode,
  XFA_ATTRIBUTE_BofAction,
  XFA_ATTRIBUTE_Save,
  XFA_ATTRIBUTE_TargetType,
  XFA_ATTRIBUTE_KeyEncipherment,
  XFA_ATTRIBUTE_CredentialServerPolicy,
  XFA_ATTRIBUTE_Size,
  XFA_ATTRIBUTE_InitialNumber,
  XFA_ATTRIBUTE_Slope,
  XFA_ATTRIBUTE_CSpace,
  XFA_ATTRIBUTE_ColSpan,
  XFA_ATTRIBUTE_Binding,
  XFA_ATTRIBUTE_Checksum,
  XFA_ATTRIBUTE_CharEncoding,
  XFA_ATTRIBUTE_Bind,
  XFA_ATTRIBUTE_TextEntry,
  XFA_ATTRIBUTE_Archive,
  XFA_ATTRIBUTE_Uuid,
  XFA_ATTRIBUTE_Posture,
  XFA_ATTRIBUTE_After,
  XFA_ATTRIBUTE_Orphans,
  XFA_ATTRIBUTE_QualifiedName,
  XFA_ATTRIBUTE_Usehref,
  XFA_ATTRIBUTE_Locale,
  XFA_ATTRIBUTE_Weight,
  XFA_ATTRIBUTE_UnderlinePeriod,
  XFA_ATTRIBUTE_Data,
  XFA_ATTRIBUTE_Desc,
  XFA_ATTRIBUTE_Numbered,
  XFA_ATTRIBUTE_DataColumnCount,
  XFA_ATTRIBUTE_Overline,
  XFA_ATTRIBUTE_UrlPolicy,
  XFA_ATTRIBUTE_AnchorType,
  XFA_ATTRIBUTE_LabelRef,
  XFA_ATTRIBUTE_BookendLeader,
  XFA_ATTRIBUTE_MaxLength,
  XFA_ATTRIBUTE_AccessKey,
  XFA_ATTRIBUTE_CursorLocation,
  XFA_ATTRIBUTE_DelayedOpen,
  XFA_ATTRIBUTE_Target,
  XFA_ATTRIBUTE_DataEncipherment,
  XFA_ATTRIBUTE_AfterTarget,
  XFA_ATTRIBUTE_Leader,
  XFA_ATTRIBUTE_Picker,
  XFA_ATTRIBUTE_From,
  XFA_ATTRIBUTE_BaseProfile,
  XFA_ATTRIBUTE_Aspect,
  XFA_ATTRIBUTE_RowColumnRatio,
  XFA_ATTRIBUTE_LineHeight,
  XFA_ATTRIBUTE_Highlight,
  XFA_ATTRIBUTE_ValueRef,
  XFA_ATTRIBUTE_MaxEntries,
  XFA_ATTRIBUTE_DataLength,
  XFA_ATTRIBUTE_Activity,
  XFA_ATTRIBUTE_Input,
  XFA_ATTRIBUTE_Value,
  XFA_ATTRIBUTE_BlankOrNotBlank,
  XFA_ATTRIBUTE_AddRevocationInfo,
  XFA_ATTRIBUTE_GenericFamily,
  XFA_ATTRIBUTE_Hand,
  XFA_ATTRIBUTE_Href,
  XFA_ATTRIBUTE_TextEncoding,
  XFA_ATTRIBUTE_LeadDigits,
  XFA_ATTRIBUTE_Permissions,
  XFA_ATTRIBUTE_SpaceAbove,
  XFA_ATTRIBUTE_CodeBase,
  XFA_ATTRIBUTE_Stock,
  XFA_ATTRIBUTE_IsNull,
  XFA_ATTRIBUTE_RestoreState,
  XFA_ATTRIBUTE_ExcludeAllCaps,
  XFA_ATTRIBUTE_FormatTest,
  XFA_ATTRIBUTE_HScrollPolicy,
  XFA_ATTRIBUTE_Join,
  XFA_ATTRIBUTE_KeyCertSign,
  XFA_ATTRIBUTE_Radius,
  XFA_ATTRIBUTE_SourceAbove,
  XFA_ATTRIBUTE_Override,
  XFA_ATTRIBUTE_ClassId,
  XFA_ATTRIBUTE_Disable,
  XFA_ATTRIBUTE_Scope,
  XFA_ATTRIBUTE_Match,
  XFA_ATTRIBUTE_Placement,
  XFA_ATTRIBUTE_Before,
  XFA_ATTRIBUTE_WritingScript,
  XFA_ATTRIBUTE_EndChar,
  XFA_ATTRIBUTE_Lock,
  XFA_ATTRIBUTE_Long,
  XFA_ATTRIBUTE_Intact,
  XFA_ATTRIBUTE_XdpContent,
  XFA_ATTRIBUTE_DecipherOnly,
};

enum class XFA_Element : int32_t {
  Unknown = -1,

  Ps,
  To,
  Ui,
  RecordSet,
  SubsetBelow,
  SubformSet,
  AdobeExtensionLevel,
  Typeface,
  Break,
  FontInfo,
  NumberPattern,
  DynamicRender,
  PrintScaling,
  CheckButton,
  DatePatterns,
  SourceSet,
  Amd,
  Arc,
  Day,
  Era,
  Jog,
  Log,
  Map,
  Mdp,
  BreakBefore,
  Oid,
  Pcl,
  Pdf,
  Ref,
  Uri,
  Xdc,
  Xdp,
  Xfa,
  Xsl,
  Zpl,
  Cache,
  Margin,
  KeyUsage,
  Exclude,
  ChoiceList,
  Level,
  LabelPrinter,
  CalendarSymbols,
  Para,
  Part,
  Pdfa,
  Filter,
  Present,
  Pagination,
  Encoding,
  Event,
  Whitespace,
  DefaultUi,
  DataModel,
  Barcode,
  TimePattern,
  BatchOutput,
  Enforce,
  CurrencySymbols,
  AddSilentPrint,
  Rename,
  Operation,
  Typefaces,
  SubjectDNs,
  Issuers,
  SignaturePseudoModel,
  WsdlConnection,
  Debug,
  Delta,
  EraNames,
  ModifyAnnots,
  StartNode,
  Button,
  Format,
  Border,
  Area,
  Hyphenation,
  Text,
  Time,
  Type,
  Overprint,
  Certificates,
  EncryptionMethods,
  SetProperty,
  PrinterName,
  StartPage,
  PageOffset,
  DateTime,
  Comb,
  Pattern,
  IfEmpty,
  SuppressBanner,
  OutputBin,
  Field,
  Agent,
  OutputXSL,
  AdjustData,
  AutoSave,
  ContentArea,
  EventPseudoModel,
  WsdlAddress,
  Solid,
  DateTimeSymbols,
  EncryptionLevel,
  Edge,
  Stipple,
  Attributes,
  VersionControl,
  Meridiem,
  ExclGroup,
  ToolTip,
  Compress,
  Reason,
  Execute,
  ContentCopy,
  DateTimeEdit,
  Config,
  Image,
  SharpxHTML,
  NumberOfCopies,
  BehaviorOverride,
  TimeStamp,
  Month,
  ViewerPreferences,
  ScriptModel,
  Decimal,
  Subform,
  Select,
  Window,
  LocaleSet,
  Handler,
  HostPseudoModel,
  Presence,
  Record,
  Embed,
  Version,
  Command,
  Copies,
  Staple,
  SubmitFormat,
  Boolean,
  Message,
  Output,
  PsMap,
  ExcludeNS,
  Assist,
  Picture,
  Traversal,
  SilentPrint,
  WebClient,
  LayoutPseudoModel,
  Producer,
  Corner,
  MsgId,
  Color,
  Keep,
  Query,
  Insert,
  ImageEdit,
  Validate,
  DigestMethods,
  NumberPatterns,
  PageSet,
  Integer,
  SoapAddress,
  Equate,
  FormFieldFilling,
  PageRange,
  Update,
  ConnectString,
  Mode,
  Layout,
  Sharpxml,
  XsdConnection,
  Traverse,
  Encodings,
  Template,
  Acrobat,
  ValidationMessaging,
  Signing,
  DataWindow,
  Script,
  AddViewerPreferences,
  AlwaysEmbed,
  PasswordEdit,
  NumericEdit,
  EncryptionMethod,
  Change,
  PageArea,
  SubmitUrl,
  Oids,
  Signature,
  ADBE_JSConsole,
  Caption,
  Relevant,
  FlipLabel,
  ExData,
  DayNames,
  SoapAction,
  DefaultTypeface,
  Manifest,
  Overflow,
  Linear,
  CurrencySymbol,
  Delete,
  Deltas,
  DigestMethod,
  InstanceManager,
  EquateRange,
  Medium,
  TextEdit,
  TemplateCache,
  CompressObjectStream,
  DataValue,
  AccessibleContent,
  NodeList,
  IncludeXDPContent,
  XmlConnection,
  ValidateApprovalSignatures,
  SignData,
  Packets,
  DatePattern,
  DuplexOption,
  Base,
  Bind,
  Compression,
  User,
  Rectangle,
  EffectiveOutputPolicy,
  ADBE_JSDebugger,
  Acrobat7,
  Interactive,
  Locale,
  CurrentPage,
  Data,
  Date,
  Desc,
  Encrypt,
  Draw,
  Encryption,
  MeridiemNames,
  Messaging,
  Speak,
  DataGroup,
  Common,
  Sharptext,
  PaginationOverride,
  Reasons,
  SignatureProperties,
  Threshold,
  AppearanceFilter,
  Fill,
  Font,
  Form,
  MediumInfo,
  Certificate,
  Password,
  RunScripts,
  Trace,
  Float,
  RenderPolicy,
  LogPseudoModel,
  Destination,
  Value,
  Bookend,
  ExObject,
  OpenAction,
  NeverEmbed,
  BindItems,
  Calculate,
  Print,
  Extras,
  Proto,
  DSigData,
  Creator,
  Connect,
  Permissions,
  ConnectionSet,
  Submit,
  Range,
  Linearized,
  Packet,
  RootElement,
  PlaintextMetadata,
  NumberSymbols,
  PrintHighQuality,
  Driver,
  IncrementalLoad,
  SubjectDN,
  CompressLogicalStructure,
  IncrementalMerge,
  Radial,
  Variables,
  TimePatterns,
  EffectiveInputPolicy,
  NameAttr,
  Conformance,
  Transform,
  LockDocument,
  BreakAfter,
  Line,
  List,
  Source,
  Occur,
  PickTrayByPDFSize,
  MonthNames,
  Severity,
  GroupParent,
  DocumentAssembly,
  NumberSymbol,
  Tagged,
  Items
};
struct XFA_ELEMENTINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_Element eName;
  uint32_t dwPackets;
  XFA_ObjectType eObjectType;
};

enum XFA_ATTRIBUTETYPE {
  XFA_ATTRIBUTETYPE_NOTSURE,
  XFA_ATTRIBUTETYPE_Enum,
  XFA_ATTRIBUTETYPE_Cdata,
  XFA_ATTRIBUTETYPE_Boolean,
  XFA_ATTRIBUTETYPE_Integer,
  XFA_ATTRIBUTETYPE_Measure,
};
struct XFA_ATTRIBUTEINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_ATTRIBUTE eName;
  XFA_ATTRIBUTETYPE eType;
  uint32_t dwPackets;
  void* pDefValue;
};

struct XFA_ELEMENTHIERARCHY {
  uint16_t wStart;
  uint16_t wCount;
};

struct XFA_SCRIPTHIERARCHY {
  uint16_t wMethodStart;
  uint16_t wMethodCount;
  uint16_t wAttributeStart;
  uint16_t wAttributeCount;
  int16_t wParentIndex;
};

#define XFA_PROPERTYFLAG_OneOf 0x01
#define XFA_PROPERTYFLAG_DefaultOneOf 0x02
struct XFA_PROPERTY {
  XFA_Element eName;
  uint8_t uOccur;
  uint8_t uFlags;
};

struct XFA_ATTRIBUTEENUMINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_ATTRIBUTEENUM eName;
};

enum XFA_UNIT {
  XFA_UNIT_Unknown,
  XFA_UNIT_Percent,
  XFA_UNIT_Angle,
  XFA_UNIT_Em,
  XFA_UNIT_Pt,
  XFA_UNIT_In,
  XFA_UNIT_Pc,
  XFA_UNIT_Cm,
  XFA_UNIT_Mm,
  XFA_UNIT_Mp,
};

struct XFA_NOTSUREATTRIBUTE {
  XFA_Element eElement;
  XFA_ATTRIBUTE attribute;
  XFA_ATTRIBUTETYPE eType;
  void* pValue;
};

typedef void (CJX_Object::*XFA_METHOD_CALLBACK)(CFXJSE_Arguments* pArguments);
struct XFA_METHODINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_METHOD_CALLBACK callback;
};

typedef void (CJX_Object::*XFA_ATTRIBUTE_CALLBACK)(CFXJSE_Value* pValue,
                                                   bool bSetting,
                                                   XFA_ATTRIBUTE eAttribute);
enum XFA_SCRIPT_TYPE {
  XFA_SCRIPT_Basic,
  XFA_SCRIPT_Object,
};

struct XFA_SCRIPTATTRIBUTEINFO {
  uint32_t uHash;
  const wchar_t* pName;
  XFA_ATTRIBUTE_CALLBACK callback;
  XFA_ATTRIBUTE attribute;
  uint16_t eValueType;
};

#endif  // XFA_FXFA_FXFA_BASIC_H_

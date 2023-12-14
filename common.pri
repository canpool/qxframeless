# eg: $$qtLibraryNameVersion(qxframeless, 0)
defineReplace(qtLibraryNameVersion) {
   RET = $$qtLibraryTarget($$1)
   win32 {
      RET = $$RET$$2
   }
   return($$RET)
}

INCLUDEPATH += $$PWD/src

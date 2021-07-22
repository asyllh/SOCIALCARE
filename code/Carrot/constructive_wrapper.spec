# -*- mode: python -*-

from PyInstaller.utils.hooks import collect_data_files # this is very helpful
from osgeo import gdal, ogr, osr
from fiona.ogrext import Iterator, ItemsIterator, KeysIterator

block_cipher = None


a = Analysis(['constructive_wrapper.py'],
             pathex=['C:\\Users\\ah4c20\\Asyl\\PostDoc\\SOCIALCARE\\code\\screpo'],
             binaries=[],
             datas=[
             ("C:\\Users\\ah4c20\Anaconda3\\Lib\\site-packages\\branca\\*.json","branca"),
             ("C:\\Users\\ah4c20\\Anaconda3\\Lib\\site-packages\\branca\\templates","templates"),
             ("C:\\Users\\ah4c20\\Anaconda3\\Lib\\site-packages\\folium\\templates","templates"),
             ],
             hiddenimports=[
                     'fiona._shim',
                     'fiona.schema'],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher,
             noarchive=False)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
        a.scripts,
        a.binaries,
        a.zipfiles,
        a.datas,
        [],
        name='constructive_wrapper',
        debug=False,
        bootloader_ignore_signals=False,
        strip=False,
        upx=True,
        runtime_tmpdir=None,
        console=True )

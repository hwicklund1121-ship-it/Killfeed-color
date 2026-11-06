"""Generate the VIP menu shell and separate Name/Prefix page layouts."""
from pathlib import Path

OUT = Path(__file__).resolve().parents[1] / 'GRZ_Killfeed/gui/layouts'
EXACT = 'hexactpos 1\nvexactpos 1\nhexactsize 1\nvexactsize 1'


def widget(kind, name, x, y, w, h, props='', children=''):
    body = f'position {x} {y}\nsize {w} {h}\n{EXACT}\n{props}\n'
    if children:
        body += '{\n' + children + '}\n'
    return f'{kind}WidgetClass {name}\n{{\n{body}}}\n'


def panel(name, x, y, w, h, color, children=''):
    return widget('Image', name, x, y, w, h, f'ignorepointer 1\nmode blend\nstretch 1\ncolor {color}', children)


def frame(name, x, y, w, h, children='', props=''):
    return widget('Frame', name, x, y, w, h, 'color 0 0 0 0\n' + props, children)


def text(name, label, x, y, w, h, color='1 1 1 1', centered=False):
    props = f'ignorepointer 1\ncolor {color}\nfont "gui/fonts/sdf_MetronLight16"\n"exact text" 1\n"text valign" center\ntext "{label}"'
    if centered:
        props += '\n"text halign" center'
    return widget('Text', name, x, y, w, h, props)


def image(name, x, y, w, h):
    return widget('Image', name, x, y, w, h, 'ignorepointer 1\ncolor 1 1 1 1\nmode blend\nstretch 1')


def button(name, label, x, y, w, h=46, red=False):
    border = '1 1 1 1' if red else '0.22 0.22 0.22 1'
    fill = '0.94 0.94 0.94 1' if red else '0.065 0.065 0.065 1'
    children = panel(name+'Border', 0, 0, w, h, border)
    children += panel(name+'Fill', 1, 1, w-2, h-2, fill)
    children += text(name+'Label', label, 12, 0, w-24, h, color='0.02 0.02 0.02 1' if red else '1 1 1 1', centered=True)
    return widget('Button', name, x, y, w, h, 'color 0 0 0 0', children)


def preview(y, height):
    children = panel('PreviewCard', 0, y, 440, height, '0.045 0.045 0.045 1')
    children += text('PreviewCaption', 'LIVE PREVIEW', 16, y+12, 408, 18, '0.65 0.65 0.65 1', True)
    baseline = 286 if y == 232 else 210
    children += image('PrefixPreviewIcon', 16, baseline-2, 40, 40)
    children += text('PrefixPreviewWord', '', 60, baseline, 160, 36)
    children += text('Preview', 'Your username', 224, baseline, 200, 36)
    return children


def main():
    shell = panel('MenuBackground', 0, 0, 1100, 760, '0 0 0 0.8')
    shell += panel('MenuTop', 0, 0, 1100, 3, '1 1 1 1')
    shell += panel('MenuBottom', 0, 759, 1100, 1, '0.35 0.35 0.38 1')
    shell += panel('MenuLeft', 0, 3, 1, 756, '0.35 0.35 0.38 1')
    shell += panel('MenuRight', 1099, 3, 1, 756, '0.35 0.35 0.38 1')
    shell += image('MenuLogo', 32, 12, 300, 100)
    shell += text('Title', 'GRZ VIP', 370, 28, 698, 42, '1 1 1 1')
    shell += text('Hint', 'Your prefix, animated killfeed and custom gun reskins.', 370, 78, 698, 24, '0.7 0.7 0.7 1')
    shell += button('NameTab', 'KILLFEED', 256, 128, 208, 48)
    shell += button('PrefixTab', 'PREFIX', 32, 128, 208, 48)
    shell += button('GunTab', 'GUN RESKINS', 480, 128, 240, 48)
    shell += panel('HeaderDivider', 32, 184, 1036, 1, '0.25 0.25 0.28 1')
    shell += frame('PageHost', 32, 208, 1036, 400)
    shell += text('Status', 'Preview your style, then save when ready.', 32, 628, 1036, 24, '0.75 0.75 0.75 1')
    shell += panel('FooterDivider', 32, 664, 1036, 1, '0.16 0.16 0.16 1')
    shell += button('Test', 'TEST FEED', 32, 686, 240, 48)
    shell += button('Close', 'CLOSE', 548, 686, 240, 48)
    shell += button('Save', 'SAVE CHANGES', 812, 686, 256, 48, True)
    # The shell owns the navigation and page host, so all move with the background.
    shell = widget('Frame', 'MenuPanel', 0, 0, 1100, 760, 'color 1 1 1 1', shell)
    (OUT/'menu.layout').write_text('FrameWidgetClass GRZKillfeedMenuRoot\n{\nvisible 1\nclipchildren 0\nhexactpos 0\nvexactpos 0\nhexactsize 0\nvexactsize 0\ncolor 0 0 0 0\nposition 0 0\nsize 1 1\n{\n'+shell+'}\n}\n')

    common = panel('PageDivider', 464, 0, 1, 360, '0.16 0.16 0.16 1')
    common += frame('AnimationHost', 496, 0, 540, 360)
    name = text('PageTitle', 'Killfeed style', 0, 0, 440, 36)
    name += text('PageDescription', 'Animate your name in the live killfeed.', 0, 48, 440, 24, '0.7 0.7 0.7 1')
    name += preview(104, 240)
    name += text('PageNote', 'Name, prefix and icon share the same animation.', 0, 362, 440, 22, '0.55 0.55 0.55 1')
    (OUT/'menu_name_page.layout').write_text(frame('GRZNamePage', 0, 0, 1036, 392, name+common))

    prefix = text('PageTitle', 'Prefix style', 0, 0, 440, 36)
    prefix += text('PageDescription', 'Add your own word and choose a custom icon.', 0, 44, 440, 24, '0.7 0.7 0.7 1')
    editor = panel('PrefixWordBacking', 0, 0, 440, 44, '0.1 0.1 0.1 1')
    editor += text('PrefixWordCaption', 'WORD', 12, 0, 64, 44, '0.65 0.65 0.65 1')
    editor += widget('EditBox', 'PrefixWordInput', 88, 5, 340, 34, 'color 1 1 1 1\nfont "gui/fonts/sdf_MetronLight16"\ntext ""')
    editor += button('PrefixIconChoice', 'Icon: None >', 0, 66, 328, 52)
    editor += panel('IconPreviewBacking', 352, 58, 88, 72, '0.045 0.045 0.045 1')
    editor += image('PrefixChoicePreview', 360, 58, 72, 72)
    prefix += frame('PrefixControls', 0, 82, 440, 130, editor)
    prefix += preview(232, 112)
    prefix += text('PageNote', 'Up to 12 letters, digits, _ or -. Leave empty to remove.', 0, 362, 440, 22, '0.55 0.55 0.55 1')
    (OUT/'menu_prefix_page.layout').write_text(frame('GRZPrefixPage', 0, 0, 1036, 392, frame('PrefixSetup', 298, 0, 440, 392, prefix), 'visible 0'))

    guns = ''
    for name,label,x in [('VIPGun20','20MM',0),('VIPGun50','.50 CAL',264),('VIPGun408','.408',528),('VIPGunOther','OTHER',792)]:
        guns += button(name,label,x,0,244,42)
    guns += panel('VIPGunListBacking',0,60,300,300,'0.06 0.06 0.07 1')
    guns += widget('TextListbox','VIPGunList',0,60,300,300,'color 1 1 1 1\nfont "gui/fonts/sdf_MetronLight16"')
    guns += panel('VIPGunPreviewBacking',324,60,712,210,'0.05 0.05 0.06 1')
    guns += widget('ItemPreview','VIPGunPreview',344,60,672,210,'ignorepointer 1\ncolor 1 1 1 1\nmode blend')
    guns += button('VIPGunPrevious','<',324,280,48,42)
    guns += text('VIPGunName','Loading reskins...',382,280,596,42,centered=True)
    guns += button('VIPGunNext','>',988,280,48,42)
    guns += text('VIPWrapHeading','GHILLIE WRAP',324,332,156,42)
    guns += button('VIPWrapPrevious','<',486,332,48,42)
    guns += text('VIPWrapName','No wrap',544,332,434,42,centered=True)
    guns += button('VIPWrapNext','>',988,332,48,42)
    guns += text('VIPGearNote','Compatible VIP suppressors are attached automatically.',324,378,712,18,'0.65 0.65 0.68 1',True)
    (OUT/'menu_guns_page.layout').write_text(frame('GRZGunPage',0,0,1036,400,guns,'visible 0'))

    animation = text('AnimationHeading', 'COLORS & ANIMATION', 0, 0, 540, 32)
    animation += button('ColorA', 'Primary: White', 0, 64, 258)
    animation += button('ColorB', 'Secondary: Red', 278, 64, 262)
    animation += panel('PrimaryActive', 0, 108, 258, 2, '1 1 1 1')
    animation += panel('SecondaryActive', 278, 108, 262, 2, '0 0 0 0')
    colors=['White','Red','Orange','Yellow','Green','Blue','Indigo','Violet','Cyan','Pink','Gray','Black']
    for i,label in enumerate(colors):
        children = panel('SwatchBorder'+str(i), 0, 0, 80, 40, '0.2 0.2 0.2 1')
        children += panel('SwatchFill'+str(i), 2, 2, 76, 36, '0 0 0 1')
        children += text('SwatchLabel'+str(i), label, 2, 2, 76, 36, centered=True)
        animation += widget('Button','Swatch'+str(i), (i%6)*92, 126+(i//6)*56, 80, 40, 'color 0 0 0 0',children)
    animation += button('Effect','Style: Steady',0,254,258)
    animation += button('Speed','Speed: Normal',278,254,262)
    animation += text('AnimationNote','Shared by your name, prefix and icon. Save when ready.',0,320,540,24,'0.6 0.6 0.6 1')
    (OUT/'menu_animation.layout').write_text(frame('AnimationControls',0,0,540,360,animation))


if __name__ == '__main__':
    main()

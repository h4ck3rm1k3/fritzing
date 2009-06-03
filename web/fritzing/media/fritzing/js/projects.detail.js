$(document).ready(function() {
    media_path = "/media/fritzing/img/";
    $('a[rel*=lightbox]').lightBox({
        imageLoading: media_path + 'lightbox-ico-loading.gif',
        imageBtnPrev: media_path + 'lightbox-btn-prev.gif',
        imageBtnNext: media_path + 'lightbox-btn-next.gif',
        imageBtnClose: media_path + 'lightbox-btn-close.gif',
        imageBlank: media_path + 'lightbox-blank.gif',
    });
});

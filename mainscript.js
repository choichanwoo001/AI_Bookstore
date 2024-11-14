self.addEventListener('fetch', function(e) {})

var slideIndex = [1, 1, 1, 1, 1];
    var slideClass = ["firstY", "secondY", "thirdY", "forthY", "etc"];
  
    showSlides(1, 0);
    showSlides(1, 1);
    showSlides(1, 2);
    showSlides(1, 3);
    showSlides(1, 4);
  
    function plusSlides(n, no) {
      showSlides(slideIndex[no] += n, no);
    }
  
    function showSlides(n, no) {
      var i;
      var slides = document.getElementsByClassName(slideClass[no])[0].getElementsByTagName("li");
      
      if (n > slides.length - 2) //앞으로 돌아가는것 
      { slideIndex[no] = 1; }    

      if (n < 1) //맨 뒤로 가는것 
      { slideIndex[no] = slides.length - 2; }
  
      for (i = 0; i < slides.length; i++) //숨겨지는 책 감추는 기능 
      { slides[i].style.display = "none"; }
      
      for (i = slideIndex[no] - 1; i < slideIndex[no] + 2; i++) {
        if (slides[i]) {
          slides[i].style.display = "inline-block";
        }
      }
    }